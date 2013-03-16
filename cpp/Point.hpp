#ifndef __POINT_H__
#define __POINT_H__
#include "mshcd.hpp"

typedef struct Point
{
	u32 x, y;
	Point()
	{
		x = y = 0;
	}
	Point(u32 x, u32 y)
	{
		this->x = x;
		this->y = y;
	}
}Point;

#endif

