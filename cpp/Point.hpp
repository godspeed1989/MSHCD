#ifndef __POINT_H__
#define __POINT_H__

typedef struct Point
{
	int x, y;
}Point;

typedef struct Rectangle
{
	int x, y;
	int w, h;
	Rectangle(int x, int y, int w, int h)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
}Rectangle;

#endif

