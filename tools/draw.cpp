#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;

int main(int argc, const char* argv[])
{
	FILE* fin;
	Point pt1, pt2;
	Scalar scalar(0, 0, 0, 0);
	unsigned int width, height;
	assert(argc>2);
	fin = fopen(argv[1], "r");
	assert(fin);
	Mat image = imread(argv[2], 1);
	while(!feof(fin))
	{
		fscanf(fin, "%d %d %d %d", &pt1.y, &pt1.x, &width, &height);
		printf("%d %d %d %d\n", pt1.x, pt1.y, width, height);
		pt2.x = pt1.x + width;
		pt2.y = pt1.y + height;
		rectangle(image, pt1, pt2, scalar, 0, 8, 0);
	}
	imshow("result", image);
	waitKey();
	return 0;
}
