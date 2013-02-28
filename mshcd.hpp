#ifndef __MSHCD_HPP__
#define __MSHCD_HPP__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
using namespace std;

typedef struct Rect
{
	unsigned int x, y;
	unsigned int width, height;
	double weight;
}Rect;
typedef struct Tree
{
	Rect rects[3];
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

