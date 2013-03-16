#include "mshcd.hpp"

MSHCD::MSHCD(const char* imagefile, const char* haarcasadefile)
{
	assert(sizeof(u8) == 1);
	assert(sizeof(u16) == 2);
	assert(sizeof(u32) == 4);
	assert(sizeof(u64) == 8);
	haarcascade.ScaleUpdate = 1.0/1.3;
	haarcascade.size1 = haarcascade.size2 =
	GetHaarCascade(haarcasadefile, haarcascade.stages); // get classifer from file
	GetIntergralImages(imagefile);  // calculate integral image
	GetIntegralCanny();             // calculate integral canny image
	HaarCasadeObjectDetection();    // start detection
	objects = merge(objects, 1);    // merge found results
	PrintDetectionResult();         // show detection result
}

/**
 * calculate intergral image from original gray-level image
 */
void MSHCD::GetIntergralImages(const char* imagefile)
{
	FILE *fin;
	u32 i, j, size;
	PRINT_FUNCTION_INFO();
	// read grayscale image data from raw image
	fin = fopen(imagefile, "rb");
	assert(fin);
	fread(&image.width, 4, 1, fin);
	fread(&image.height, 4, 1, fin);
	printf("%d X %d\n", image.width, image.height);
	size = image.width*image.height;
	image.data = (u8*)malloc(size*sizeof(u8));
	fread(image.data, image.width*image.height, 1, fin);
	fclose(fin);
	
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
void MSHCD::HaarCasadeObjectDetection()
{
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
	printf("Start iteration.....\n");
	for(i=0; i<itt; ++i)
	{
		printf("itt %d ", i+1);
		Scale = StartScale * pow(haarcascade.ScaleUpdate, itt-i);
		step = (Scale>2.0) ? (u32)Scale : 2;
		printf("scale %lf step %d ", Scale, step);
		width = (u32)(haarcascade.size1 * Scale);
		height = (u32)(haarcascade.size2 * Scale);
		printf("width %d height %d ", width, height);
		// Make vectors with all search image coordinates 
		Point point;
		vector<Point> points;
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
					if( d<20 || d>100 )
						continue;
				}
				point.x = x, point.y = y;
				points.push_back(point);
			}
		}
		OneScaleObjectDetection(points, Scale, width, height);
	}
	PRINT_FUNCTION_END_INFO();
}

double MSHCD::GetSumRect(u8 type, u32 x, u32 y, u32 w, u32 h)
{
	if(w==0 || h==0)
		return 0;
	w--; h--;
	//printf("(%d %d) %d %d\n", x, y, w, h);
	assert(x+w<image.width && y+h<image.height);
	if(type == II1)
		return	*( image.idata1+(y+h)*image.width+(x+w) )
			-	*( image.idata1+(y+0)*image.width+(x+w) )
			-	*( image.idata1+(y+h)*image.width+(x+0) )
			+	*( image.idata1+(y+0)*image.width+(x+0) );
	if(type == II2)
		return	*( image.idata2+(y+h)*image.width+(x+w) )
			-	*( image.idata2+(y+0)*image.width+(x+w) )
			-	*( image.idata2+(y+h)*image.width+(x+0) )
			+	*( image.idata2+(y+0)*image.width+(x+0) );
	assert(0);
	return 0;
}

/**
 * detect the object at a fixed scale, fixed width, fixed height
 */
void MSHCD::OneScaleObjectDetection(vector<Point> points, double Scale,
                                    u32 width, u32 height)
{
	u32 i, i_stage, i_tree, max;
	//PRINT_FUNCTION_INFO();
	//printf("*****Total %d rects to find\n", points.size());
	for(i=0; i<points.size(); i++)
	{
		u8 pass = 1;
		for(i_stage=0; i_stage<haarcascade.stages.size(); i_stage++)
		{
			DPRINTF("----Stage %d----", i_stage);
			Stage &stage = haarcascade.stages[i_stage];
			double StageSum =  0.0;
			for(i_tree=0; i_tree<stage.trees.size(); i_tree++)
			{
				Tree &tree = stage.trees[i_tree];
				DPRINTF("----Tree %d----\n", i_tree);
				StageSum += TreeObjectDetection(tree, Scale, points[i], width, height);
			}
			if(StageSum > stage.threshold)
				continue;
			else
			{
				pass = 0;
				break;
			}
		}
		if(i_stage > max) max = i_stage;
		if(pass)
		{
			Rectangle rect;
			rect.x = points[i].x;
			rect.y = points[i].y;
			rect.width = width;
			rect.height = height;
			rect.weight = -1;
			objects.push_back(rect);
			printf("+");
		}
	}
	printf("\n");
	//PRINT_FUNCTION_END_INFO();
}

/**
 * get the current haar-classifier's value (left/right)
 */
double MSHCD::TreeObjectDetection(Tree& tree, double Scale, Point& point,
                                  u32 width, u32 height)
{
	u32 i_rect, x, y;
	double InverseArea;
	u32 total_x, total_x2;
	double moy, vnorm, Rectangle_sum;
	x = point.x;
	y = point.y;
	//PRINT_FUNCTION_INFO();
	// get the variance of pixel values in the window
	InverseArea = 1.0/(width*height);
	//#define GET_SUM
	#ifdef GET_SUM
	total_x = GetSumRect(II1, point.x, point.y, width, height);
	total_x2 = GetSumRect(II2, point.x, point.y, width, height);
	#else
	total_x = image(II1, x+width, y+height) + image(II1, x, y)
			- image(II1, x+width, y) - image(II1, x, y+height);
	total_x2 = image(II2, x+width, y+height) + image(II2, x, y)
			- image(II2, x+width, y) - image(II2, x, y+height);
	#endif
	moy = total_x * InverseArea;
	vnorm = total_x2 * InverseArea - moy * moy;
	vnorm = (vnorm > 1.0) ? sqrt(vnorm) : 1.0;
	// for each rectangle in the feature
	Rectangle_sum = 0.0;
	for(i_rect=0; i_rect<3; i_rect++)
	{
		Rectangle &rect = tree.rects[i_rect];
		//printf("[%d %d] %d %d\n", rect.x, rect.y, rect.width, rect.height);
	#ifdef GET_SUM
		u32 RectX = rect.x * Scale + x;
		u32 RectY = rect.y * Scale + y;
		u32 RectWidth = rect.width * Scale;
		u32 RectHeight = rect.height * Scale;
		Rectangle_sum += GetSumRect(II1, RectX, RectY, RectWidth, RectHeight) * rect.weight;
	#else
		u32 rx1 = x + (u32) (Scale * rect.x);
		u32 rx2 = x + (u32) (Scale * (rect.x+rect.width));
		u32 ry1 = y + (u32) (Scale * rect.y);
		u32 ry2 = y + (u32) (Scale * (rect.y+rect.height));
		Rectangle_sum += (image(II1, rx2, ry2) + image(II1, rx1, ry1)
						- image(II1, rx2, ry1) - image(II1, rx1, ry2)) * rect.weight;
	#endif
	}
	Rectangle_sum *= InverseArea;
	if(Rectangle_sum < tree.threshold*vnorm)
		return tree.left_val;
	else
		return tree.right_val;
}

void MSHCD::GetIntegralCanny()
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
	u32 *grad = (u32*)malloc(image.width*image.height*(sizeof(u32)));
	for(i=1; i<image.width-1; i++)
	{
		for(j=1; j<image.height-1; j++)
		{
			u32 grad_x, grad_y;
			grad_x = -image(CANNY,i-1,j-1)+image(CANNY,i+1,j-1)-2*image(CANNY,i-1,j)+
					2*image(CANNY,i+1,j)-image(CANNY,i-1,j+1)+image(CANNY,i+1,j+1);
			grad_y = image(CANNY,i-1,j-1)+2*image(CANNY,i,j-1)+image(CANNY,i+1,j-1)-
					image(CANNY,i-1,j+1)-2*image(CANNY,i,j+1)-image(CANNY,i+1,j+1);
			*(grad+j*image.width+i) = abs(grad_x) + abs(grad_y);
		}
	}
	/* Suppression of non-maxima of the gradient and computation of the integral Canny image. */
	for(i=0; i<image.width; i++)
	{
		u32 col=0;
		for(j=0; j<image.height; j++)
		{
			col += *(grad+j*image.width+i);
			image(CANNY, i, j) = (i>0?image(CANNY,i-1,j):0) + col;
		}
	}
	free(grad);
}

/** Returns true if two rectangles overlap */
bool equals(Rectangle& r1, Rectangle& r2)
{
	u32 dist_x, dist_y;
	dist_x = (u32)(r1.width * 0.05);
	dist_y = (u32)(r1.height * 0.05);
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

vector<Rectangle> MSHCD::merge(vector<Rectangle> objs, u32 min_neighbors)
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

void MSHCD::PrintDetectionResult()
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

