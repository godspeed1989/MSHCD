#include "Detector.hpp"
#include <stdio.h>

int main()
{
	const char * imagefile = "../tools/gray_img.raw";
	const char * cascadefile = "../haar.txt";
	FILE *fin;
	Image image;
	unsigned char* data;
	unsigned long i, size;
	
	fin = fopen(imagefile, "rb");
	assert(fin);
	fscanf(fin, "%d %d\n", &image.w, &image.h);
	printf("%d X %d\n", image.w, image.h);
	size = image.w*image.h;
	data = (unsigned char*)malloc(size*sizeof(unsigned char));
	fread(data, size, 1, fin);
	fclose(fin);
	
	image.data = (unsigned int*)malloc(size*sizeof(unsigned int));
	for(i=0; i<size; i++)
	{
		image.data[i] = data[i];
	}
	
	Detector detect(cascadefile);
	detect.getFaces(image, 1, 1.25f, 0.1f);
	
	return 0;
}
