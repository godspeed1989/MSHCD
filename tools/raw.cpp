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
	fclose(fout);
	return 0;
}
