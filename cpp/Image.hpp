#ifndef __IMAGE_H__
#define __IMAGE_H__
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mshcd.hpp"

typedef struct Image
{
	u32 *data;
	u32 width, height;
	Image()
	{
		this->width = this->height = 0;
		this->data = NULL;
	}
	Image(u32 w, u32 h)
	{
		this->width = w;
		this->height = h;
		this->data = (u32*)malloc(w*h*sizeof(u32));
		assert(this->data);
	}
	Image(Image& image)
	{
		this->width = image.width;
		this->height = image.height;
		this->data = (u32*)malloc(image.width*image.height*sizeof(u32));
		assert(this->data);
		memcpy(this->data, image.data, image.width*image.height);
	}
	~Image()
	{
		if(data)
			free(data);
	}
	void init(u32 w, u32 h)
	{
		if(data)
			free(data);
		this->width = w;
		this->height = h;
		this->data = (u32*)malloc(w*h*sizeof(u32));
		assert(data);
	}
	u32 getWidth()
	{
		return this->width;
	}
	u32 getHeight()
	{
		return this->height;
	}
	u32 getSum(u32 x, u32 y,
						u32 w, u32 h)
	{
		if(x+w<width && y+h<height)
		{
			return	*( data+(y+h)*width+(x+w) )
				+	*( data+(y+0)*width+(x+0) )
				-	*( data+(y+h)*width+(x+0) )
				-	*( data+(y+0)*width+(x+w) );
		}
		else
		{
			printf("Image: (%d+%d, %d+%d)\n", x, y, w, h);
			printf("Image: %d %d\n", width, height);
			assert(0);
		}
	}
	u32& operator () (u32 x, u32 y)
	{
		assert(data);
		assert(x<width && y<height);
		return *(data+y*width+x);
	}
}Image;

#endif

