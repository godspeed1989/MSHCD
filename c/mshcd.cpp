#include "mshcd.hpp"
#ifdef WITH_OPENCV
#include <opencv2/opencv.hpp>
#endif

void MSHCD(HAAR *m, const char* imagefile, const char* haarcasadefile)
{
	assert(sizeof(u8) == 1);
	assert(sizeof(u16) == 2);
	assert(sizeof(u32) == 4);
	assert(sizeof(u64) == 8);
	m->haarcascade.ScaleUpdate = 1.0/1.14;
	m->haarcascade.size1 = m->haarcascade.size2 = \
	GetHaarCascade(haarcasadefile, m->haarcascade.stages); // get classifer from file
	GetIntergralImages(imagefile, m->image);               // calculate integral image
	GetIntegralCanny(m->image);                            // calculate integral canny image
	HaarCasadeObjectDetection(m);                          // start detection
	m->objects = MergeRects(m->objects, 1);                // merge found results
	PrintDetectionResult(m->objects);                      // show detection result
}

/**
 * calculate intergral image from original gray-level image
 */
void GetIntergralImages(const char* imagefile, Image &image)
{
	u32 i, j, size;
	PRINT_FUNCTION_INFO();
#ifndef WITH_OPENCV
	FILE *fin;
	printf("read grayscale image data from raw image.\n");
	fin = fopen(imagefile, "rb");
	assert(fin);
	fread(&image.width, 4, 1, fin);
	fread(&image.height, 4, 1, fin);
	printf("%d X %d\n", image.width, image.height);
	size = image.width*image.height;
	image.data = (u8*)malloc(size*sizeof(u8));
	fread(image.data, size*sizeof(u8), 1, fin);
	fclose(fin);
#else
	cv::Mat img;
	img = cv::imread(imagefile, 0);
	image.width = img.cols;
	image.height = img.rows;
	printf("%d X %d\n", image.width, image.height);
	size = image.width*image.height;
	image.data = (u8*)malloc(size*sizeof(u8));
	memcpy(image.data, img.data, size*sizeof(u8));
#endif
	
	image.idata1 = (u32*)malloc(size*sizeof(u32));
	memset(image.idata1, 0, size*sizeof(u32));
	image.idata2 = (u32*)malloc(size*sizeof(u32));
	memset(image.idata2, 0, size*sizeof(u32));
	for(i=0; i<image.width; i++)
	{
		u32 col = 0.0;
		u32 col2 = 0.0;
		for(j=0; j<image.height; j++)
		{
			u32 idx = j*image.width + i;
			u8 value = *(image.data + idx);	
			col += value;
			col2 += value*value;
			if(i>0)
			{
				*(image.idata1 + idx) = *(image.idata1 + idx - 1);
				*(image.idata2 + idx) = *(image.idata2 + idx - 1);
			}
			*(image.idata1 + idx) += col;
			*(image.idata2 + idx) += col2;
		}
	}
	/*for(i=0; i<10; i++)
	{
		for(j=0; j<10; j++)
			printf("%d ", (int)*(image.data+j*image.width+i));
		printf("\n");
	}*/
	PRINT_FUNCTION_END_INFO();
}

/**
 * Dectect object use Multi-scale Haar cascade classifier
 */
void HaarCasadeObjectDetection(HAAR *m)
{
	Image &image = m->image;
	HaarCascade &haarcascade = m->haarcascade;
	double Scale, StartScale, ScaleWidth, ScaleHeight;
	u32 i, itt, x, y, width, height, step;
	PRINT_FUNCTION_INFO();
	// get start scale
	ScaleWidth = image.width/haarcascade.size1;
	ScaleHeight = image.height/haarcascade.size2;
	if(ScaleHeight < ScaleWidth)
		StartScale = ScaleHeight; 
	else
		StartScale = ScaleWidth;
	printf("StartScale = %lf\n", StartScale);
	itt = (u32)ceil(log(1.0/StartScale)/log(haarcascade.ScaleUpdate));
	printf("Total itt = %lf / %lf = %d\n", \
			log(1.0/StartScale), log(haarcascade.ScaleUpdate), itt);
	printf("Start detection.....\n");
	for(i=0; i<itt; ++i)
	{
		printf("itt %d ", i+1);
		Scale = StartScale * pow(haarcascade.ScaleUpdate, itt-i);
		step = (Scale>2.0) ? (u32)Scale : 2;
		printf("scale %lf step %d ", Scale, step);
		width = (u32)(haarcascade.size1 * Scale);
		height = (u32)(haarcascade.size2 * Scale);
		printf("width %d height %d ", width, height);
		//0:step:(IntegralImages.width-width-1)
		//0:step:(IntegralImages.height-height-1)
		for(x=0; x<image.width-width; x+=step)
		{
			for(y=0; y<image.height-height; y+=step)
			{
				if(1)
				{
					u32 edges_density, d;
					edges_density = image(CANNY,x+width,y+height)+image(CANNY,x,y)-
									image(CANNY,x,y+height)-image(CANNY,x+width,y);
					d = edges_density/(width*height);
					if( d<20 || d>150 )
						continue;
				}
				Point pnt(x,y);
				OneScaleObjectDetection(m, pnt, Scale, width, height);
			}
		}
		printf("\n");
	}
	PRINT_FUNCTION_END_INFO();
}

/**
 * detect the object at a fixed scale, fixed width, fixed height
 */
void OneScaleObjectDetection(HAAR *m, Point& point, double Scale,
                             u32 width, u32 height)
{
	HaarCascade &haarcascade = m->haarcascade;
	u32 i_stage, i_tree;
	u8 pass = TRUE;
	//PRINT_FUNCTION_INFO();
	for(i_stage=0; i_stage<haarcascade.stages.size(); i_stage++)
	{
		DPRINTF("----Stage %d----", i_stage);
		Stage &stage = haarcascade.stages[i_stage];
		double StageSum =  0.0;
		for(i_tree=0; i_tree<stage.trees.size(); i_tree++)
		{
			Tree &tree = stage.trees[i_tree];
			DPRINTF("----Tree %d----\n", i_tree);
			StageSum += TreeObjectDetection(m, tree, Scale, point, width, height);
		}
		if(StageSum > stage.threshold)
			continue;
		else
		{
			pass = FALSE;
			break;
		}
	}
	if(pass == TRUE)
	{
		Rectangle rect;
		rect.x = point.x;
		rect.y = point.y;
		rect.width = width;
		rect.height = height;
		rect.weight = -1;
		m->objects.push_back(rect);
		printf("+");
	}
	//PRINT_FUNCTION_END_INFO();
}

/**
 * get the current haar-classifier's value (left/right)
 */
double TreeObjectDetection(HAAR *m, Tree& tree, double Scale, Point& point,
                           u32 width, u32 height)
{
	Image &image = m->image;
	u32 i_rect;
	double InverseArea;
	u32 total_x, total_x2;
	double moy, vnorm, Rectangle_sum;
	u32 &x = point.x;
	u32 &y = point.y;
	//PRINT_FUNCTION_INFO();
	// get the variance of pixel values in the window
	InverseArea = 1.0/(width*height);
	
	total_x = image(II1, x+width, y+height) + image(II1, x, y)
	        - image(II1, x+width, y) - image(II1, x, y+height);
	total_x2 = image(II2, x+width, y+height) + image(II2, x, y)
	         - image(II2, x+width, y) - image(II2, x, y+height);
	
	moy = total_x * InverseArea;
	vnorm = total_x2 * InverseArea - moy * moy;
	vnorm = (vnorm > 1.0) ? sqrt(vnorm) : 1.0;
	// for each rectangle in the feature
	Rectangle_sum = 0.0;
	for(i_rect=0; i_rect<tree.nb_rects; i_rect++)
	{
		Rectangle &rect = tree.rects[i_rect];
		//printf("[%d %d] %d %d\n", rect.x, rect.y, rect.width, rect.height);
		u32 rx1 = x + (u32) (Scale * rect.x);
		u32 rx2 = x + (u32) (Scale * (rect.x+rect.width));
		u32 ry1 = y + (u32) (Scale * rect.y);
		u32 ry2 = y + (u32) (Scale * (rect.y+rect.height));
		Rectangle_sum += (image(II1, rx2, ry2) + image(II1, rx1, ry1)
						- image(II1, rx2, ry1) - image(II1, rx1, ry2)) * rect.weight;
	}
	Rectangle_sum *= InverseArea;
	if(Rectangle_sum < tree.threshold*vnorm)
		return tree.left_val;
	else
		return tree.right_val;
}

void GetIntegralCanny(Image &image)
{
	u32 i, j;
	u32 sum;
	image.cdata = (u32*)malloc(image.width*image.height*(sizeof(u32)));
	for(i=2; i<image.width-2; i++)
	{
		for(j=2; j<image.height-2; j++)
		{
			sum = 0;
			sum += 2  * image(i-2, j-2);
			sum += 4  * image(i-2, j-1);
			sum += 5  * image(i-2, j+0);
			sum += 4  * image(i-2, j+1);
			sum += 2  * image(i-2, j+2);
			sum += 4  * image(i-1, j-2);
			sum += 9  * image(i-1, j-1);
			sum += 12 * image(i-1, j+0);
			sum += 9  * image(i-1, j+1);
			sum += 4  * image(i-1, j+2);
			sum += 5  * image(i+0, j-2);
			sum += 12 * image(i+0, j-1);
			sum += 15 * image(i+0, j+0);
			sum += 12 * image(i+0, j+1);
			sum += 5  * image(i+0, j+2);
			sum += 4  * image(i+1, j-2);
			sum += 9  * image(i+1, j-1);
			sum += 12 * image(i+1, j+0);
			sum += 9  * image(i+1, j+1);
			sum += 4  * image(i+1, j+2);
			sum += 2  * image(i+2, j-2);
			sum += 4  * image(i+2, j-1);
			sum += 5  * image(i+2, j+0);
			sum += 4  * image(i+2, j+1);
			sum += 2  * image(i+2, j+2);
			image(CANNY, i, j) = sum / 159;
		}
	}
	/*Computation of the discrete gradient of the image.*/
	long grad_x, grad_y;
	u32 *grad = (u32*)malloc(image.width*image.height*(sizeof(u32)));
	for(i=1; i<image.width-1; i++)
	{
		for(j=1; j<image.height-1; j++)
		{
			grad_x = -image(CANNY,i-1,j-1)+image(CANNY,i+1,j-1)-2*image(CANNY,i-1,j)+
					2*image(CANNY,i+1,j)-image(CANNY,i-1,j+1)+image(CANNY,i+1,j+1);
			grad_y = image(CANNY,i-1,j-1)+2*image(CANNY,i,j-1)+image(CANNY,i+1,j-1)-
					image(CANNY,i-1,j+1)-2*image(CANNY,i,j+1)-image(CANNY,i+1,j+1);
			*(grad+j*image.width+i) = abs(grad_x) + abs(grad_y);
		}
	}
	/* Suppression of non-maxima of the gradient and computation of the integral Canny image. */
	u32 col;
	for(i=0; i<image.width; i++)
	{
		col = 0;
		for(j=0; j<image.height; j++)
		{
			col += *(grad+j*image.width+i);
			image(CANNY, i, j) = (i>0?image(CANNY,i-1,j):0) + col;
		}
	}
	free(grad);
}

/** Returns true if two rectangles overlap */
static bool equals(Rectangle& r1, Rectangle& r2)
{
	u32 dist_x, dist_y;
	dist_x = (u32)(r1.width * 0.15);
	dist_y = (u32)(r1.height * 0.15);
	/* y - distance <= x <= y + distance */
	if(	r2.x <= r1.x + dist_x &&
		r2.x >= r1.x - dist_x &&
		r2.y <= r1.y + dist_y &&
		r2.y >= r1.y - dist_y &&
		(r2.width <= r1.width + dist_x || r2.width >= r1.width - dist_x) && 
		(r2.height <= r1.height + dist_y || r2.height >= r1.height - dist_y) )
		return true;
	if(	r1.x>=r2.x && r1.x+r1.width<=r2.x+r2.width &&
		r1.y>=r2.y && r1.y+r1.height<=r2.y+r2.height )
		return true;
	if(	r2.x>=r1.x && r2.x+r2.width<=r1.x+r1.width &&
		r2.y>=r1.y && r2.y+r2.height<=r1.y+r1.height )
		return true;
	return false;
}

vector<Rectangle> MergeRects(vector<Rectangle> objs, u32 min_neighbors)
{
	vector<Rectangle> ret;
	u32 i, j;
	u32 *mark = (u32*)malloc(objs.size()*sizeof(u32));
	u32 nb_classes = 0;
	/* mark each rectangle with a class number */
	for(i=0; i<objs.size(); i++)
	{
		u8 found = FALSE;
		for(j=0; j<i; j++)
		{
			if(equals(objs[i], objs[j]))
			{
				found = TRUE;
				mark[i] = mark[j];
			}
		}
		if(!found)
		{
			mark[i] = nb_classes;
			nb_classes++;
		}
	}
	u32 *neighbors = (u32*)malloc(nb_classes*sizeof(u32));
	Rectangle *rects = (Rectangle*)malloc(nb_classes*sizeof(Rectangle));
	for(i=0; i<nb_classes; i++)
	{
		neighbors[i] = 0;
		rects[i].x = rects[i].y = rects[i].width = rects[i].height = 0;
	}
	/* calculate number of rects of each class */
	for(i=0; i<objs.size(); i++)
	{
		neighbors[mark[i]]++;
		rects[mark[i]].x += objs[i].x;
		rects[mark[i]].y += objs[i].y;
		rects[mark[i]].width += objs[i].width;
		rects[mark[i]].height += objs[i].height;
	}
	for(i=0; i<nb_classes; i++)
	{
		u32 n = neighbors[i];
		if(n >= min_neighbors)
		{
			Rectangle r;
			r.x = (rects[i].x*2 + n)/(2*n);
			r.y = (rects[i].y*2 + n)/(2*n);
			r.width = (rects[i].width*2 + n)/(2*n);
			r.height = (rects[i].height*2 + n)/(2*n);
			ret.push_back(r);
		}
	}
	free(mark);
    free(neighbors);
	free(rects);
	return ret;
}

void PrintDetectionResult(vector<Rectangle> &objects)
{
	FILE *fout;
	u32 i_obj;
	PRINT_FUNCTION_INFO();
	printf("Total %d object(s) found\n", objects.size());
	fout = fopen("result.txt", "w");
	for(i_obj=0; i_obj<objects.size(); i_obj++)
	{
		Rectangle &rect = objects[i_obj];
		fprintf(fout, "%d %d %d %d\n", rect.x, rect.y, rect.width, rect.height);
	}
	fclose(fout);
	PRINT_FUNCTION_END_INFO();
}

#ifdef WITH_OPENCV
void ShowDetectionResult(const char* file, vector<Rectangle> &objects)
{
	u32 i_obj;
	cv::Point pt1, pt2;
	cv::Mat image = cv::imread(file, 1);
	cv::Scalar scalar(255, 255, 0, 0);
	if(objects.size() == 0)
		return;
	for(i_obj=0; i_obj<objects.size(); i_obj++)
	{
		Rectangle &rect = objects[i_obj];
		pt1.x = rect.x;
		pt1.y = rect.y;
		pt2.x = pt1.x + rect.width;
		pt2.y = pt1.y + rect.height;
		rectangle(image, pt1, pt2, scalar, 3, 8, 0);
	}
	cv::imshow("result", image);
	cv::waitKey();
	cv::imwrite("result.jpg", image);
}
#endif

