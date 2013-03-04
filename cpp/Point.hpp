#ifndef __POINT_H__
#define __POINT_H__

typedef struct Point
{
	unsigned int x, y;
	Point()
	{
		x = y = 0;
	}
	Point(unsigned int x, unsigned int y)
	{
		this->x = x;
		this->y = y;
	}
}Point;

typedef struct Rectangle
{
	unsigned int x, y;
	unsigned int width, height;
	Rectangle()
	{
		x = y = width = height = 0;
	}
	Rectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
	{
		this->x = x;
		this->y = y;
		this->width = w;
		this->height = h;
	}
}Rectangle;

#endif

