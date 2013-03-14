#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main(int argc, const char* argv[])
{
	assert(argc>1);
	
	// read & show grayscale original image
	Mat image = imread(argv[1], 0);
	cout<<image.cols<<" X "<<image.rows<<endl;
	cout<<(image.depth()==CV_8U)<<" * "<<image.channels()<<" "<<endl;
	imshow("original", image);
	waitKey();
	// write out the `raw` image
	FILE *fout;
	fout = fopen("gray_img.raw", "wb");
	fwrite(&image.cols, 4, 1, fout);
	fwrite(&image.rows, 4, 1, fout);
	for(int i=0; i<image.rows*image.cols; i++)
	{
		fprintf(fout, "%c", image.data[i]);
	}
	fflush(fout);
	fclose(fout);
	// test read & show the `raw` image
	FILE *fin;
	int rows, cols;
	unsigned char *ptr;
	fin = fopen("gray_img.raw", "rb");
	fread(&cols, 4, 1, fin);
	fread(&rows, 4, 1, fin);
	cout<<cols<<" X "<<rows<<endl;
	ptr = (unsigned char*)malloc(rows*cols);
	fread(ptr, rows*cols, 1, fin);
	Mat img(rows, cols, CV_8U, ptr);
	imshow("test in", img);
	waitKey();
	fclose(fin);
	
	return 0;
}

