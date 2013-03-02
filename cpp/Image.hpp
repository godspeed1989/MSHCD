#ifndef __IMAGE_H__
#define __IMAGE_H__
#include <assert.h>
#include <stdlib.h>

typedef struct Image
{
	unsigned int *data;
	int w, h;
	Image()
	{
		w = h = 0;
		this->data = NULL;
	}
	Image(int w, int h)
	{
		this->w = w;
		this->h = h;
		this->data = (unsigned int*)malloc(w*h*sizeof(unsigned int));
	}
	~Image()
	{
		if(data)
			free(data);
	}
	int getWidth()
	{
		return this->w;
	}
	int getHeight()
	{
		return this->h;
	}
	unsigned int& operator () (int i, int j)
	{
		assert(data);
		return *(data+j*w+j);
	}
}Image;

#endif

