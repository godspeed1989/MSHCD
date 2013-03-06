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

#endif

