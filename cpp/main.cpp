#include <cstdio>
#include "Detector.hpp"
#include "mshcd.hpp"
#ifdef WITH_OPENCV
#include <opencv2/opencv.hpp>
#endif

int main(int argc, const char* argv[])
{
	FILE *fout;
	u32 w, h;
	u8* data;
	u32 i, size;
	if(argc < 3)
	{
		printf("Usage: %s image cascade\n", argv[0]);
		return -1;
	}
#ifndef WITH_OPENCV
	FILE* fin;
	fin = fopen(argv[1], "rb");
	assert(fin);
	fread(&w, 4, 1, fin);
	fread(&h, 4, 1, fin);
	size = w*h;
	data = (u8*)malloc(size*sizeof(u8));
	fread(data, size, 1, fin);
	fclose(fin);
#else
	cv::Mat img = cv::imread(argv[1], 0);
	w = img.cols;
	h = img.rows;
	size = w*h;
	data = (u8*)malloc(size*sizeof(u8));
	memcpy(data, img.data, size*sizeof(u8));
#endif
	printf("%d X %d\n", w, h);
	
	Image image(w,h);
	for(i=0; i<size; i++)
	{
		image.data[i] = (u32)data[i];
	}
	free(data);
	
	Detector detect(argv[2]);
	vector<Rectangle> obj = detect.getObjects(image, 1.2, 1.3f, 0.1f, 1, 1);
	printf("%d objects after merge\n", obj.size());
	// write out
	fout = fopen("result.txt", "w");
	for(i=0; i<obj.size(); i++)
	{
		fprintf(fout, "%d %d %d %d\n", obj[i].x, obj[i].y, obj[i].width, obj[i].height);
		printf("%d %d %d %d\n", obj[i].x, obj[i].y, obj[i].width, obj[i].height);
	}
	fclose(fout);
#ifdef WITH_OPENCV
	// show detect result
	u32 i_obj;
	cv::Point pt1, pt2;
	cv::Mat orig = cv::imread(argv[1], 1);
	cv::Scalar scalar(255, 255, 0, 0);
	if(obj.size() == 0)
		return 0;
	for(i_obj=0; i_obj<obj.size(); i_obj++)
	{
		Rectangle &rect = obj[i_obj];
		pt1.x = rect.x;
		pt1.y = rect.y;
		pt2.x = pt1.x + rect.width;
		pt2.y = pt1.y + rect.height;
		rectangle(orig, pt1, pt2, scalar, 3, 8, 0);
	}
	cv::imshow("result", orig);
	cv::waitKey();
	cv::imwrite("result.jpg", orig);
#endif
	return 0;
}
