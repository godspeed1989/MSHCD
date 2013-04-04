#include <cstdio>
#include "Detector.hpp"
#include "mshcd.hpp"
#ifdef WITH_OPENCV
#include <opencv2/opencv.hpp>
#endif

int main()
{
#ifndef WITH_OPENCV
	const char * imagefile = "../tools/gray_img.raw";
#else
	const char * imagefile = "../tools/gray_img.jpg";
#endif
	const char * cascadefile = "../haar_alt.txt";
	FILE *fout;
	u32 w, h;
	u8* data;
	u32 i, size;
#ifndef WITH_OPENCV
	FILE* fin;
	fin = fopen(imagefile, "rb");
	assert(fin);
	fread(&w, 4, 1, fin);
	fread(&h, 4, 1, fin);
	size = w*h;
	data = (u8*)malloc(size*sizeof(u8));
	fread(data, size, 1, fin);
	fclose(fin);
#else
	cv::Mat img;
	cv::Point pt1, pt2;
	cv::Scalar scalar(0, 0, 0, 0);
	img = cv::imread(imagefile, 0);
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
	
	Detector detect(cascadefile);
	vector<Rectangle> obj = detect.getObjects(image, 1.2, 1.2f, 0.1f, 1, 1);
	printf("Number objects %d after merge\n", obj.size());
	fout = fopen("result.txt", "w");
	for(i=0; i<obj.size(); i++)
	{
		fprintf(fout, "%d %d %d %d\n", obj[i].x, obj[i].y, obj[i].width, obj[i].height);
		printf("%d %d %d %d\n", obj[i].x, obj[i].y, obj[i].width, obj[i].height);
		#ifdef WITH_OPENCV
		pt1.x = obj[i].x;
		pt1.y = obj[i].y;
		pt2.x = pt1.x + obj[i].width;
		pt2.y = pt1.y + obj[i].height;
		rectangle(img, pt1, pt2, scalar, 3, 8, 0);
		#endif
	}
	fclose(fout);
#ifdef WITH_OPENCV
	cv::imshow("result", img);
	cv::waitKey();
	cv::imwrite("result.jpg", img);
#endif
	return 0;
}

