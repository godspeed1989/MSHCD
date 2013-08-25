#include "mshcd.h"
#ifdef WITH_OPENCV
#include <cv.h>
#include <highgui.h>
#endif

static inline u32 I(Image *img, int type, u32 x, u32 y)
{
	switch(type)
	{
		case GRAY:  return *(img->data   + y*img->width + x);
		case II1:   return *(img->idata1 + y*img->width + x);
		case II2:   return *(img->idata2 + y*img->width + x);
		case CANNY: return *(img->cdata  + y*img->width + x);
		default:  printf("Error: Unknow Image Type!\n");
	}
	exit(-1);
	return -1;
}

void MSHCD(HAAR *m, const char* imagefile, const char* haarcasadefile)
{
	assert(sizeof(u8) == 1);
	assert(sizeof(u16) == 2);
	assert(sizeof(u32) == 4);
	assert(sizeof(u64) == 8);

	m->n_objects = 0;
	m->objects = (Rectangle**)malloc(MAX_OBJECTS * sizeof(Rectangle*));
	m->haarcascade.ScaleUpdate = 1.0/1.3;

	GetHaarCascade(haarcasadefile, &m->haarcascade); // get classifer from file
	GetIntergralImages(imagefile, &m->image);        // calculate integral image
#ifdef DO_CANNY_PURGE
	GetIntegralCanny(&m->image);    // calculate integral canny image
#endif

	HaarCasadeObjectDetection(m);   // start detection

	MergeRects(m, 1);               // merge found results
	PrintDetectionResult(m);        // show detection result

#ifdef WITH_OPENCV
	ShowDetectionResult(imagefile, m);
#endif
}

/**
 * Clean up function
 */
void MSHCD_Cleanup(HAAR *m)
{
	u32 i, j;
	for(i = 0; i < m->haarcascade.n_stages; ++i)
	{
		Stage *stage = m->haarcascade.stages[i];
		for(j = 0; j < stage->n_trees; ++j)
		{
			free(stage->trees[j]);
		}
		free(stage->trees);
		free(stage);
	}
	free(m->haarcascade.stages);
	for(i = 0; i < m->n_objects; ++i)
		free(m->objects[i]);
	free(m->objects);
}

/**
 * calculate intergral image from original gray-level image
 */
void GetIntergralImages(const char* imagefile, Image *image)
{
	u32 i, j, size;
	PRINT_FUNCTION_INFO();
#ifndef WITH_OPENCV
	FILE *fin;
	printf("read grayscale image data from raw image.\n");
	fin = fopen(imagefile, "rb");
	assert(fin);
	fread(&image->width, 4, 1, fin);
	fread(&image->height, 4, 1, fin);
#else
	IplImage* img;
	img = cvLoadImage(imagefile, 0);
	image->width = img->width;
	image->height = img->height;
#endif
	printf("%d X %d\n", image->width, image->height);
	size = image->width * image->height;
	image->data = (u8*)malloc(size * sizeof(u8));
#ifndef WITH_OPENCV
	fread(image->data, size * sizeof(u8), 1, fin);
	fclose(fin);
#else
	memcpy(image->data, img->imageData, size * sizeof(u8));
#endif
	image->idata1 = (u32*)malloc(size*sizeof(u32));
	memset(image->idata1, 0, size*sizeof(u32));
	image->idata2 = (u32*)malloc(size*sizeof(u32));
	memset(image->idata2, 0, size*sizeof(u32));
	for(i = 0; i < image->width; ++i)
	{
		u32 col = 0.0;
		u32 col2 = 0.0;
		for(j = 0; j < image->height; ++j)
		{
			u32 idx = j*image->width + i;
			u8 value = *(image->data + idx);	
			col += value;
			col2 += value*value;
			if(i>0)
			{
				*(image->idata1 + idx) = *(image->idata1 + idx - 1);
				*(image->idata2 + idx) = *(image->idata2 + idx - 1);
			}
			*(image->idata1 + idx) += col;
			*(image->idata2 + idx) += col2;
		}
	}
	/*for(i = 0; i < 10; i++)
	{
		for(j = 0; j < 10; j++)
			printf("%d ", (int)*(image->data + j*image->width + i));
		printf("\n");
	}*/
	PRINT_FUNCTION_END_INFO();
}

/**
 * Dectect object use Multi-scale Haar cascade classifier
 */
void HaarCasadeObjectDetection(HAAR *m)
{
	Image *image = &m->image;
	HaarCascade *haarcascade = &m->haarcascade;
	double Scale, StartScale, ScaleWidth, ScaleHeight;
	u32 i, itt, x, y, width, height, step;
	PRINT_FUNCTION_INFO();
	// get start scale
	ScaleWidth = image->width / haarcascade->size1;
	ScaleHeight = image->height / haarcascade->size2;
	if(ScaleHeight < ScaleWidth)
		StartScale = ScaleHeight; 
	else
		StartScale = ScaleWidth;
	printf("StartScale = %lf\n", StartScale);
	itt = (u32)ceil(log(1.0/StartScale) / log(haarcascade->ScaleUpdate));
	printf("Total itt = %lf / %lf = %d\n", log(1.0/StartScale), log(haarcascade->ScaleUpdate), itt);
	printf("Start detection.....\n");
	Rectangle rect;
	for(i = 0; i < itt; ++i)
	{
		printf("itt %d ", i+1);
		Scale = StartScale * pow(haarcascade->ScaleUpdate, itt-i);
		step = (Scale > 2.0) ? (u32)Scale : 2;
		printf("scale %lf step %d ", Scale, step);
		width = (u32)(haarcascade->size1 * Scale);
		height = (u32)(haarcascade->size2 * Scale);
		printf("width %d height %d ", width, height);
		//0:step:(IntegralImages.width-width-1)
		//0:step:(IntegralImages.height-height-1)
		for(x = 0; x < image->width-width; x += step)
		{
			for(y = 0; y < image->height-height; y += step)
			{
			#ifdef DO_CANNY_PURGE
				u32 edges_density, d;
				edges_density = I(image,CANNY,x+width,y+height)+I(image,CANNY,x,y)-
								I(image,CANNY,x,y+height)-I(image,CANNY,x+width,y);
				d = edges_density / (width*height);
				if( d < 20 || d > 150 )
					continue;
			#endif
				rect.x = x;
				rect.y = y;
				rect.width = width;
				rect.height = height;
				FixedScaleObjectDetection(m, &rect, Scale);
			}
		}
		printf("\n");
	}
	PRINT_FUNCTION_END_INFO();
}

/**
 * detect the object at a fixed scale, fixed width, fixed height
 */
void FixedScaleObjectDetection(HAAR *m, Rectangle *rect, double Scale)
{
	u8 pass = TRUE;
	u32 i_stage, i_tree;
	HaarCascade *haarcascade = &m->haarcascade;
	//PRINT_FUNCTION_INFO();
	for(i_stage = 0; i_stage < haarcascade->n_stages; i_stage++)
	{
		DPRINTF("----Stage %d----", i_stage);
		Stage *stage = haarcascade->stages[i_stage];
		double StageSum =  0.0;
		for(i_tree = 0; i_tree < stage->n_trees; i_tree++)
		{
			DPRINTF("----Tree %d----\n", i_tree);
			StageSum += TreeObjectDetection(m, stage->trees[i_tree], rect, Scale);
		}
		if(StageSum > stage->threshold)
			continue;
		else
		{
			pass = FALSE;
			break;
		}
	}
	if(pass == TRUE)
	{
		Rectangle *r;
		if(m->n_objects >= MAX_OBJECTS)
		{
			printf("exceed max objects\n");
			return;
		}
		r = (Rectangle*)malloc(sizeof(Rectangle));
		*r = *rect;
		m->objects[m->n_objects++] = r;
		printf("+");
	}
	//PRINT_FUNCTION_END_INFO();
}

/**
 * get the current haar-classifier's value (left/right)
 */
double TreeObjectDetection(HAAR *m, Tree* tree, Rectangle *rect, double Scale)
{
	Image *image = &m->image;
	u32 i_rect;
	double InverseArea;
	u32 total_x, total_x2;
	double moy, vnorm, Rectangle_sum;
	u32 x = rect->x;
	u32 y = rect->y;
	u32 width = rect->width;
	u32 height = rect->height;
	//PRINT_FUNCTION_INFO();
	// get the variance of pixel values in the window
	InverseArea = 1.0 / (rect->width * rect->height);
	
	total_x = I(image, II1, x+width, y+height) + I(image, II1, x, y)
	        - I(image, II1, x+width, y) - I(image, II1, x, y+height);
	total_x2 = I(image, II2, x+width, y+height) + I(image, II2, x, y)
	         - I(image, II2, x+width, y) - I(image, II2, x, y+height);
	
	moy = total_x * InverseArea;
	vnorm = total_x2 * InverseArea - moy * moy;
	vnorm = (vnorm > 1.0) ? sqrt(vnorm) : 1.0;
	// for each rectangle in the feature
	Rectangle_sum = 0.0;
	for(i_rect = 0; i_rect < tree->nb_rects; i_rect++)
	{
		Rectangle *r = &tree->rects[i_rect];
		//printf("[%d %d] %d %d\n", rect.x, rect.y, rect.width, rect.height);
		u32 rx1 = x + (u32) (Scale * (r->x));
		u32 rx2 = x + (u32) (Scale * (r->x+r->width));
		u32 ry1 = y + (u32) (Scale * (r->y));
		u32 ry2 = y + (u32) (Scale * (r->y+r->height));
		Rectangle_sum += r->weight * (I(image, II1, rx2, ry2) + I(image, II1, rx1, ry1)
						            - I(image, II1, rx2, ry1) - I(image, II1, rx1, ry2));
	}
	Rectangle_sum *= InverseArea;
	if(Rectangle_sum < tree->threshold*vnorm)
		return tree->left_val;
	else
		return tree->right_val;
}

#ifdef DO_CANNY_PURGE
void GetIntegralCanny(Image *image)
{
	u32 i, j;
	u32 sum;
	image->cdata = (u32*)malloc(image->width * image->height * (sizeof(u32)));
	for(i = 2; i < image->width-2; ++i)
	{
		for(j = 2; j < image->height-2; ++j)
		{
			sum = 0;
			sum += 2  * I(image, GRAY, i-2, j-2);
			sum += 4  * I(image, GRAY, i-2, j-1);
			sum += 5  * I(image, GRAY, i-2, j+0);
			sum += 4  * I(image, GRAY, i-2, j+1);
			sum += 2  * I(image, GRAY, i-2, j+2);
			sum += 4  * I(image, GRAY, i-1, j-2);
			sum += 9  * I(image, GRAY, i-1, j-1);
			sum += 12 * I(image, GRAY, i-1, j+0);
			sum += 9  * I(image, GRAY, i-1, j+1);
			sum += 4  * I(image, GRAY, i-1, j+2);
			sum += 5  * I(image, GRAY, i+0, j-2);
			sum += 12 * I(image, GRAY, i+0, j-1);
			sum += 15 * I(image, GRAY, i+0, j+0);
			sum += 12 * I(image, GRAY, i+0, j+1);
			sum += 5  * I(image, GRAY, i+0, j+2);
			sum += 4  * I(image, GRAY, i+1, j-2);
			sum += 9  * I(image, GRAY, i+1, j-1);
			sum += 12 * I(image, GRAY, i+1, j+0);
			sum += 9  * I(image, GRAY, i+1, j+1);
			sum += 4  * I(image, GRAY, i+1, j+2);
			sum += 2  * I(image, GRAY, i+2, j-2);
			sum += 4  * I(image, GRAY, i+2, j-1);
			sum += 5  * I(image, GRAY, i+2, j+0);
			sum += 4  * I(image, GRAY, i+2, j+1);
			sum += 2  * I(image, GRAY, i+2, j+2);
			*(image->cdata + j*image->width + i) = sum / 159;
		}
	}
	/*Computation of the discrete gradient of the image.*/
	long grad_x, grad_y;
	u32 *grad = (u32*)malloc(image->width * image->height * (sizeof(u32)));
	for(i = 1; i < image->width-1; ++i)
	{
		for(j = 1; j < image->height-1; ++j)
		{
			grad_x = -I(image, CANNY,i-1,j-1)+I(image, CANNY,i+1,j-1)-2*I(image, CANNY,i-1,j)+
					2*I(image, CANNY,i+1,j)-I(image, CANNY,i-1,j+1)+I(image, CANNY,i+1,j+1);
			grad_y = I(image, CANNY,i-1,j-1)+2*I(image, CANNY,i,j-1)+I(image, CANNY,i+1,j-1)-
					I(image, CANNY,i-1,j+1)-2*I(image, CANNY,i,j+1)-I(image, CANNY,i+1,j+1);
			*(grad+j*image->width+i) = abs(grad_x) + abs(grad_y);
		}
	}
	/* Suppression of non-maxima of the gradient and computation of the integral Canny image. */
	u32 col;
	for(i = 0; i < image->width; ++i)
	{
		col = 0;
		for(j = 0; j < image->height; ++j)
		{
			col += *(grad+j*image->width+i);
			*(image->cdata + j*image->width + i) = (i>0?I(image, CANNY,i-1,j):0) + col;
		}
	}
	free(grad);
}
#endif

/** Returns true if two rectangles overlap */
static int equals(Rectangle* r1, Rectangle* r2)
{
	u32 dist_x, dist_y;
	dist_x = (u32)(r1->width * 0.15);
	dist_y = (u32)(r1->height * 0.15);
	/* y - distance <= x <= y + distance */
	if(	r2->x <= r1->x + dist_x &&
		r2->x >= r1->x - dist_x &&
		r2->y <= r1->y + dist_y &&
		r2->y >= r1->y - dist_y &&
		(r2->width <= r1->width + dist_x || r2->width >= r1->width - dist_x) && 
		(r2->height <= r1->height + dist_y || r2->height >= r1->height - dist_y) )
		return 1;
	if(	r1->x>=r2->x && r1->x+r1->width<=r2->x+r2->width &&
		r1->y>=r2->y && r1->y+r1->height<=r2->y+r2->height )
		return 1;
	if(	r2->x>=r1->x && r2->x+r2->width<=r1->x+r1->width &&
		r2->y>=r1->y && r2->y+r2->height<=r1->y+r1->height )
		return 1;
	return 0;
}

void MergeRects(HAAR *m, u32 min_neighbors)
{
	u32 i, j;
	u32 *mark = (u32*)malloc(m->n_objects * sizeof(u32));
	u32 nb_classes = 0;
	/* mark each rectangle with a class number */
	for(i = 0; i < m->n_objects; ++i)
	{
		u8 found = FALSE;
		for(j = 0; j < i; ++j)
		{
			if(equals(m->objects[i], m->objects[j]))
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
	u32 *neighbors = (u32*)malloc(nb_classes * sizeof(u32));
	Rectangle **rects = (Rectangle**)malloc(MAX_OBJECTS * sizeof(Rectangle*));
	for(i = 0; i < nb_classes; ++i)
	{
		neighbors[i] = 0;
		rects[i] = (Rectangle*)malloc(sizeof(Rectangle));
		memset(rects[i], 0, sizeof(Rectangle));
	}
	/* cumulate the rects of each class */
	for(i = 0; i < m->n_objects; ++i)
	{
		neighbors[mark[i]]++;
		rects[mark[i]]->x += m->objects[i]->x;
		rects[mark[i]]->y += m->objects[i]->y;
		rects[mark[i]]->width += m->objects[i]->width;
		rects[mark[i]]->height += m->objects[i]->height;
	}
	/* caculate the position of objects */
	for(i = 0; i < nb_classes; ++i)
	{
		u32 n = neighbors[i];
		if(n >= min_neighbors)
		{
			rects[i]->x = (rects[i]->x*2 + n)/(2*n);
			rects[i]->y = (rects[i]->y*2 + n)/(2*n);
			rects[i]->width = (rects[i]->width*2 + n)/(2*n);
			rects[i]->height = (rects[i]->height*2 + n)/(2*n);
		}
	}
	/* overlap the original storage */
	for(i = 0; i < m->n_objects; ++i)
		free(m->objects[i]);
	free(m->objects);
	m->objects = rects;
	m->n_objects = nb_classes;

	free(mark);
    free(neighbors);
}

void PrintDetectionResult(HAAR *m)
{
	FILE *fout;
	u32 i_obj;
	PRINT_FUNCTION_INFO();
	printf("Total %d object(s) found\n", m->n_objects);
	fout = fopen("result.txt", "w");
	for(i_obj = 0; i_obj < m->n_objects; i_obj++)
	{
		Rectangle *rect = m->objects[i_obj];
		fprintf(fout, "%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);
	}
	fclose(fout);
	PRINT_FUNCTION_END_INFO();
}

#ifdef WITH_OPENCV
void ShowDetectionResult(const char* file, HAAR *m)
{
	u32 i_obj;
	CvPoint pt1, pt2;
	IplImage* image = cvLoadImage(file, 1);
	if(m->n_objects == 0)
		return;
	for(i_obj = 0; i_obj < m->n_objects; i_obj++)
	{
		Rectangle *rect = m->objects[i_obj];
		pt1.x = rect->x;
		pt1.y = rect->y;
		pt2.x = pt1.x + rect->width;
		pt2.y = pt1.y + rect->height;
		cvRectangle(image, pt1, pt2, cvScalar(255, 255, 0, 0), 3, 8, 0);
	}
	cvShowImage("result", image);
	cvWaitKey(0);
	cvSaveImage("result.jpg", image, 0);
	cvReleaseImage(&image);
}
#endif

