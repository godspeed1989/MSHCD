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
		if(this->data)
			free(this->data);
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
		assert(this->data);
		return *(this->data+j*this->w+j);
	}
}Image;

#endif

