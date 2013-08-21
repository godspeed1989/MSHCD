#ifndef __MSHCD_HPP__
#define __MSHCD_HPP__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TRUE  1
#define FALSE 0
typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;

#define MAX_STAGES              50
#define MAX_TREES_PER_STAGE     800
#define MAX_OBJECTS             150

typedef struct Rectangle
{
	u32 x, y;
	u32 width, height;
	double weight;
}Rectangle;
typedef struct Tree
{
	Rectangle rects[3];
	u32 tilted;
	u32 nb_rects;
	double threshold;
	double left_val, right_val;
}Tree;
typedef struct Stage
{
	double threshold;
	Tree **trees;
	u32 n_trees;
}Stage;
typedef struct HaarCascade
{
	u32 size1, size2;
	double ScaleUpdate;
	Stage **stages;
	u32 n_stages;
}HaarCascade;

enum
{
	GRAY, II1, II2, CANNY
};
typedef struct Image
{
	u32 width, height;
	u8* data;
	u32 *idata1;
	u32 *idata2;
	u32 *cdata;
}Image;

inline u32 I(Image *img, int type, u32 x, u32 y)
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

#define PRINT_FUNCTION_INFO() printf("------%s()\n", __FUNCTION__)
#define PRINT_FUNCTION_END_INFO() printf("%s()------\n", __FUNCTION__)
//#define DEBUG
#ifdef DEBUG
#define DPRINTF(args...) printf(args)
#else
#define DPRINTF(args...)
#endif

extern void GetHaarCascade(const char* filename, HaarCascade* haarcascade);

#ifdef __cplusplus
 // #define WITH_OPENCV
#endif
#define DO_CANNY_PURGE

typedef struct HAAR
{
	Image image;
	HaarCascade haarcascade;	
	Rectangle **objects;
	u32 n_objects;
}HAAR;

void MSHCD(HAAR *m, const char* imagefile, const char* haarcasadefile);

void GetIntergralImages(const char* imagefile, Image *image);
#ifdef DO_CANNY_PURGE
void GetIntegralCanny(Image *image);
#endif

void HaarCasadeObjectDetection(HAAR *m);
void FixedScaleObjectDetection(HAAR *m, Rectangle *rect, double Scale);
double TreeObjectDetection(HAAR *m, Tree* tree, Rectangle *rect, double Scale);

void MergeRects(HAAR *m, u32 min_neighbors);

void PrintDetectionResult(HAAR *m);
#ifdef WITH_OPENCV
void ShowDetectionResult(const char* file, HAAR *m);
#endif

void MSHCD_Cleanup(HAAR *m);

#endif

