#ifndef __MSHCD_HPP__
#define __MSHCD_HPP__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
using namespace std;

typedef struct Rectangle
{
	unsigned int x, y;
	unsigned int width, height;
	double weight;
}Rectangle;
typedef struct Tree
{
	Rectangle rects[3];
	int tilted;
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
	unsigned int size1, size2;
	double ScaleUpdate;
	vector<Stage> stages;
}HaarCascade;

#define II1 1
#define II2 2
typedef struct Image
{
	unsigned int width, height;
	unsigned char* data;
	double* idata1;
	double* idata2;
	double operator () (int type, unsigned int x, unsigned int y)
	{
		assert(x<width && y<height);
		if(type == II1)
			return *(idata1 + y*width + x);
		if(type == II2)
			return *(idata2 + y*width + x);
		return 0;
	}
}Image;

typedef struct Point
{
	unsigned int x, y;
}Point;

#define PRINT_FUNCTION_INFO() printf("------%s()\n", __FUNCTION__)
#define PRINT_FUNCTION_END_INFO() printf("%s()------\n", __FUNCTION__)

#ifdef DEBUG
#define DPRINTF(args...) printf(args)
#else
#define DPRINTF(args...)
#endif

#endif

