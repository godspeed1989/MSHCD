#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main(int argc, const char* argv[])
{
	assert(argc>1);

	Mat image = imread(argv[1], 0);
	cout<<image.rows<<" X "<<image.cols<<endl;
	cout<<(image.depth()==CV_8U)<<" * "<<image.channels()<<" "<<endl;
	imshow("original", image);
	waitKey();

	FILE *fout;
	fout = fopen("gray_img.raw", "wb");
	fprintf(fout, "%d %d\n", image.rows, image.cols);
	for(int i=0; i<image.rows*image.cols; i++)
	{
		fprintf(fout, "%c", image.data[i]);
	}
	fflush(fout);
	fclose(fout);

	FILE *fin;
	int rows, cols;
	unsigned char *ptr;
	fin = fopen("gray_img.raw", "rb");
	fscanf(fin, "%d %d\n", &rows, &cols);
	cout<<rows<<" X "<<cols<<endl;
	ptr = (unsigned char*)malloc(rows*cols);
	fread(ptr, rows*cols, 1, fin);
	Mat img(rows, cols, CV_8U, ptr);
	imshow("test in", img);
	waitKey();
	fclose(fin);
	return 0;
}

