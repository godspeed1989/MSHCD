#ifndef __MSHCD_HPP__
#define __MSHCD_HPP__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
using namespace std;

#define TRUE  1
#define FALSE 0
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
		return *(idata1 + y*width + x);
	}
}Image;

typedef struct Point
{
	Point(u32 _x=0, u32 _y=0):x(_x), y(_y){}
	u32 x, y;
}Point;

#define PRINT_FUNCTION_INFO() printf("------%s()\n", __FUNCTION__)
#define PRINT_FUNCTION_END_INFO() printf("%s()------\n", __FUNCTION__)

#ifdef DEBUG
#define DPRINTF(args...) printf(args)
#else
#define DPRINTF(args...)
#endif

extern u32 GetHaarCascade(const char* filename, vector<Stage>& Stages);

typedef struct MSHCD
{
	HaarCascade haarcascade;
	Image image;
	vector<Rectangle> objects;
	explicit MSHCD(const char* imagefile, const char* haarcasadefile);
	void GetIntergralImages(const char* imagefile);
	void HaarCasadeObjectDetection();
	void OneScaleObjectDetection(Point point, double Scale,
		                         u32 width, u32 height);
	double TreeObjectDetection(Tree& tree, double Scale, Point& point,
		                       u32 width, u32 height);
	void PrintDetectionResult();
	vector<Rectangle> merge(vector<Rectangle> objs, u32 min_neighbors);
	void GetIntegralCanny();
}MSHCD;

#endif

