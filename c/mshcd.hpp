#ifndef __MSHCD_HPP__
#define __MSHCD_HPP__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
using namespace std;

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;

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
	vector<Tree> trees;
}Stage;
typedef struct HaarCascade
{
	u32 size1, size2;
	double ScaleUpdate;
	vector<Stage> stages;
}HaarCascade;

#define II1    1
#define II2    2
#define CANNY  3
typedef struct Image
{
	u32 width, height;
	u8* data;
	double *idata1;
	double *idata2;
	double *cdata;
	u8& operator () (u32 x, u32 y)
	{
		return *(data + y*width + x);
	}
	double& operator () (u8 type, u32 x, u32 y)
	{
		assert(x<width && y<height);
		if(type == II1)
			return *(idata1 + y*width + x);
		if(type == II2)
			return *(idata2 + y*width + x);
		if(type == CANNY)
			return *(cdata + y*width + x);
	}
}Image;

typedef struct Point
{
	u32 x, y;
}Point;

#define PRINT_FUNCTION_INFO() printf("------%s()\n", __FUNCTION__)
#define PRINT_FUNCTION_END_INFO() printf("%s()------\n", __FUNCTION__)

#ifdef DEBUG
#define DPRINTF(args...) printf(args)
#else
#define DPRINTF(args...)
#endif

extern int GetHaarCascade(const char* filename, vector<Stage>& Stages);

typedef struct MSHCD
{
	HaarCascade haarcascade;
	Image image;
	vector<Rectangle> objects;
	explicit MSHCD(const char* imagefile, const char* haarcasadefile);
	void GetIntergralImages(const char* imagefile);
	void HaarCasadeObjectDetection();
	void OneScaleObjectDetection(vector<Point> points, double Scale,
		                         u32 w, u32 h);
	double TreeObjectDetection(Tree& tree, double Scale, Point& point,
		                       u32 width, u32 height);
	void PrintDetectionResult();
	void GetIntegralCanny();
	double GetSumRect(u8 type, u32 x, u32 y, u32 w, u32 h);
}MSHCD;

#endif

