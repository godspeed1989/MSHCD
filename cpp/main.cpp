#include "Detector.hpp"
#include <stdio.h>
#include "mshcd.hpp"

int main()
{
	const char * imagefile = "../tools/gray_img.raw";
	const char * cascadefile = "../trees.txt";
	FILE *fin, *fout;
	u32 w, h;
	u8* data;
	u32 i, size;
	
	fin = fopen(imagefile, "rb");
	assert(fin);
	fread(&w, 4, 1, fin);
	fread(&h, 4, 1, fin);
	printf("%d X %d\n", w, h);
	size = w*h;
	data = (u8*)malloc(size*sizeof(u8));
	fread(data, size, 1, fin);
	fclose(fin);
	
	Image image(w,h);
	for(i=0; i<size; i++)
	{
		image.data[i] = (u32)data[i];
	}
	free(data);
	
	Detector detect(cascadefile);
	vector<Rectangle> obj = detect.getObjects(image, 1, 1.2f, 0.15f, 1, 1);
	printf("Number objects %d after merge\n", obj.size());
	fout = fopen("result.txt", "w");
	for(i=0; i<obj.size(); i++)
	{
		fprintf(fout, "%d %d %d %d\n", obj[i].x, obj[i].y, obj[i].width, obj[i].height);
		printf("%d %d %d %d\n", obj[i].x, obj[i].y, obj[i].width, obj[i].height);
	}
	fclose(fout);
	
	return 0;
}
