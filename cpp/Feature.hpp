#ifndef __FEATURE_H__
#define __FEATURE_H__

#include <vector>
#include <math.h>
#include "Rect.hpp"
#include "Point.hpp"
#include "Image.hpp"
using namespace std;
/** 
 * A feature of the detector. A feature is located at a node of a tree, and returns LEFT or RIGHT depending on the comparison
 * of the computed value and a threshold. The value is computed as follows : the feature is constituted of weighted rectangles, and the value
 * is the sum of the pixels inside each rectangle, weighted by the rectangle's weight.
 */
#define LEFT   0
#define RIGHT  1
typedef struct Feature
{
	Rect rects[3];
	int nb_rects;
	double threshold;
	double left_val;
	double right_val;
	int left_node;
	int right_node;
	bool has_left_val;
	bool has_right_val;
	unsigned int w, h;
	int tilted;
	
	Feature()
	{
		w = h = nb_rects = 0;
		left_node = right_node = 0;
		has_left_val = has_right_val = false;
	}
	Feature(double threshold,
			double left_val, int left_node, bool has_left_val,
			double right_val, int right_node, bool has_right_val)
	{
		w = h = nb_rects = 0;
		this->threshold = threshold;
		this->left_val = left_val;
		this->left_node = left_node;
		this->has_left_val = has_left_val;
		this->right_val = right_val;
		this->right_node = right_node;
		this->has_right_val = has_right_val;
	}

	int getLeftOrRight(Image& grayImage, Image& squares,
						unsigned int x, unsigned int y, double scale)
	{
		/* Compute the area of the window.*/
		double inv_area = 1.0/(w*h);
		
		/* Compute the sum (and squared sum) of the pixel values in the window, 
		 * and get the mean and variance of pixel values
		 * in the window. */
	#define GET_SUM
	#ifdef GET_SUM
		unsigned int total_x = grayImage.getSum(x, y, w, h);
		unsigned int total_x2 = squares.getSum(x, y, w, h);
	#else
		unsigned int total_x = grayImage(x+w,y+h)+grayImage(x,y)-grayImage(x+w,y)-grayImage(x,y+h);
		unsigned int total_x2 = squares(x+w,y+h)+squares(x,y)-squares(x+w,y)-squares(x,y+h);
	#endif
		double moy = total_x*inv_area;
		double vnorm = total_x2*inv_area - moy*moy;
		vnorm = (vnorm>1.0)?sqrt(vnorm):1.0;

		double rect_sum = 0;
		/* For each rectangle in the feature. */
		for(int k=0; k<nb_rects; k++)
		{
			Rect& rect = this->rects[k];
			/* Scale the rectangle according to the window size. */
		#ifdef GET_SUM
			unsigned int RectX = rect.x * scale + x;
			unsigned int RectY = rect.y * scale + y;
			unsigned int RectWidth = rect.width * scale;
			unsigned int RectHeight = rect.height * scale;
			/* Add the sum of pixel values in the rectangles 
			 * (weighted by the rectangle's weight) to the total sum */
			rect_sum += (double)grayImage.getSum(RectX, RectY, RectWidth, RectHeight)*rect.weight;
		#else
			unsigned int rx1 = x+(unsigned int) (scale * rect.x);
			unsigned int rx2 = x+(unsigned int) (scale * (rect.x+rect.width));
			unsigned int ry1 = y+(unsigned int) (scale * rect.y);
			unsigned int ry2 = y+(unsigned int) (scale * (rect.y+rect.height));
			/* Add the sum of pixel values in the rectangles (weighted by the rectangle's weight) to the total sum */
			rect_sum += (double)((grayImage(rx2,ry2)+grayImage(rx1,ry1)-grayImage(rx1,ry2)-grayImage(rx2,ry1))*rect.weight);
		#endif
		}
		rect_sum *= inv_area;
		/* Return LEFT or RIGHT depending on how the total sum compares to the threshold. */
		return (rect_sum<threshold*vnorm)?LEFT:RIGHT;
	}

	void addRect(Rect r)
	{
		this->rects[nb_rects++] = r;
		assert(nb_rects<=3);
	}
	
	void print()
	{
		rects[0].print();
		rects[1].print();
		rects[2].print();
		printf("{L: %lf %d %d}\n", left_val, has_left_val, left_node);
		printf("{R: %lf %d %d}\n", right_val, has_right_val, right_node);
	}
}Feature;

#endif

