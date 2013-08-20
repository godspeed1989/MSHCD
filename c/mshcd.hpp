#ifndef __MSHCD_HPP__
#define __MSHCD_HPP__

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
using namespace std;

#define TRUE  1
#define FALSE 0
typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;

#define MAX_STAGES              50
#define MAX_TREES_PER_STAGE     800

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

#define II1    1
#define II2    2
#define CANNY  3
typedef struct Image
{
	u32 width, height;
	u8* data;
	u32 *idata1;
	u32 *idata2;
	u32 *cdata;
	u8& operator () (u32 x, u32 y)
	{
		return *(data + y*width + x);
	}
	u32& operator () (u8 type, u32 x, u32 y)
	{
		assert(x<width && y<height);
		if(type == II1)
			return *(idata1 + y*width + x);
		if(type == II2)
			return *(idata2 + y*width + x);
		if(type == CANNY)
			return *(cdata + y*width + x);
		else
			printf("Error: Unknow Image Type!\n");
		return *(idata1 + y*width + x);
	}
}Image;

#define PRINT_FUNCTION_INFO() printf("------%s()\n", __FUNCTION__)
#define PRINT_FUNCTION_END_INFO() printf("%s()------\n", __FUNCTION__)
//#define DEBUG
#ifdef DEBUG
#define DPRINTF(args...) printf(args)
#else
#define DPRINTF(args...)
#endif

extern void GetHaarCascade(const char* filename, HaarCascade* haarcascade);

#define WITH_OPENCV

typedef struct HAAR
{
	HaarCascade haarcascade;
	Image image;
	vector<Rectangle> objects;
}HAAR;

void MSHCD(HAAR *m, const char* imagefile, const char* haarcasadefile);

void GetIntergralImages(const char* imagefile, Image &image);
void GetIntegralCanny(Image &image);

void HaarCasadeObjectDetection(HAAR *m);
void OneScaleObjectDetection(HAAR *m, Rectangle *rect, double Scale);
double TreeObjectDetection(HAAR *m, Tree* tree, Rectangle *rect, double Scale);

vector<Rectangle> MergeRects(vector<Rectangle> objs, u32 min_neighbors);

void PrintDetectionResult(vector<Rectangle> &objects);
#ifdef WITH_OPENCV
void ShowDetectionResult(const char* file, vector<Rectangle> &objects);
#endif

void MSHCD_Cleanup(HAAR *m);

#endif

