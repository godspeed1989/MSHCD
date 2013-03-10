#ifndef __IMAGE_H__
#define __IMAGE_H__
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Image
{
	unsigned int *data;
	unsigned int width, height;
	Image()
	{
		this->width = this->height = 0;
		this->data = NULL;
	}
	Image(unsigned int w, unsigned int h)
	{
		this->width = w;
		this->height = h;
		this->data = (unsigned int*)malloc(w*h*sizeof(unsigned int));
		assert(this->data);
	}
	Image(Image& image)
	{
		this->width = image.width;
		this->height = image.height;
		this->data = (unsigned int*)malloc(image.width*image.height*sizeof(unsigned int));
		assert(this->data);
		memcpy(this->data, image.data, image.width*image.height);
	}
	~Image()
	{
		if(data)
			free(data);
	}
	int getWidth()
	{
		return this->width;
	}
	int getHeight()
	{
		return this->height;
	}
	unsigned int getSum(unsigned int x, unsigned int y,
						unsigned int w, unsigned int h)
	{
		printf("%s\n", __FUNCTION__);
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
	unsigned int& operator () (unsigned int x, unsigned int y)
	{
		assert(data);
		assert(x<width && y<height);
		return *(data+y*width+x);
	}
}Image;

#endif

