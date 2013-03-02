#ifndef __RECT_H__
#define __RECT_H__
/**
 * A simple class describing a rectangle, along with a weight.
 */
typedef struct Rect
{
	int x1, x2, y1, y2;
	double weight;
	Rect()
	{
		this->x1 = 0;
		this->x2 = 0;
		this->y1 = 0;
		this->y2 = 0;
		this->weight = 0;
	}
	Rect(int x1,int x2,int y1,int y2,double weight)
	{
		this->x1 = x1;
		this->x2 = x2;
		this->y1 = y1;
		this->y2 = y2;
		this->weight = weight;
	}
}Rect;

#endif

