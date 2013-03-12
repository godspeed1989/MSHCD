#ifndef __DETECTOR_H__
#define __DETECTOR_H__

#include <vector>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "Stage.hpp"
#include "Point.hpp"
#include "CannyPruner.hpp"
using namespace std;

int GetHaarCascade(const char* filename, vector<Stage>& Stages);

typedef struct Detector
{
	Point size;
	vector<Stage> stages;
	
	Detector(const char* filename)
	{
		size.x = size.y = GetHaarCascade(filename, stages);
		printf("Trained at %d x %d\n", size.x, size.y);
	}

	/**
	 * Returns the list of detected objects in an image applying the Viola-Jones algorithm.
	 * The algorithm tests, from sliding windows on the image, of variable size, 
	 * which regions should be considered as searched objects.
	 * Please see Wikipedia for a description of the algorithm.
	 * @param file The image file to scan.
	 * @param baseScale The initial ratio between the window size and the Haar classifier size (default 2).
	 * @param scale_inc The scale increment of the window size, at each step (default 1.25).
	 * @param increment The shift of the window at each sub-step, in terms of percentage of the window size.
	 */
	vector<Rectangle> getFaces(Image& image, double baseScale,
	                           double scale_inc, double increment,
	                           unsigned int min_neighbors, int canny_pruning)
	{
		vector<Rectangle> objects;
		unsigned int x, y, k;
		unsigned int width = image.getWidth();
		unsigned int height = image.getHeight();
		/* Compute the max scale of the detector, i.e. the size of the image divided by the size of the detector. */
		double maxScale = min((width+0.0f)/size.x, (height+0.0f)/size.y);
			
		printf("Computing integral image and squared integral image.\n");
		Image integral(width, height);
		Image squares(width, height);
		for(x=0; x<width; x++)
		{
			unsigned int col = 0;
			unsigned int col2 = 0;
			for(y=0; y<height; y++)
			{
				unsigned int value = image(x, y);
				col += value;
				col2 += value*value;
				integral(x, y) = (x>0?integral(x-1,y):0) + col;
				squares(x, y) = (x>0?squares(x-1,y):0) + col2;
			}
		}
		/*for(int i=0; i<10; i++)
		{
			for(int j=0; j<10; j++)
				printf("%d ", grayImage(i,j));
			printf("\n");
		}*/
		
		printf("Start to detection...\n");
		CannyPruner pruner;
		if(canny_pruning)
		{
			printf("Get integral canny to do canny prune.\n");
			pruner.getIntegralCanny(image);
		}
		/*For each scale of the detection window */
		double scale;
		for(scale=baseScale; scale<maxScale; scale*=scale_inc)
		{
			/*Compute the sliding step of the window*/
			int step = (int) (scale * size.x * increment);
			unsigned int w = (scale * size.x);
			unsigned int h = (scale * size.y);
			printf("*****Scale %lf step %d width %d height %d ", scale, step, w, h);
			/*For each position of the window on the image
			  check whether the object is detected there.*/
			for(x=0; x<width-w; x+=step)
			{
				for(y=0; y<height-h; y+=step)
				{
					if(canny_pruning)
					{
						unsigned int edges_density, d;
						Image& canny = pruner.canny;
						edges_density = canny(x+w,y+h)+canny(x,y)-canny(x,y+h)-canny(x+w,y);
						d = edges_density/(w*h);
						if( d<20 || d>100 )
							continue;
					}
					bool pass = true;
					/* Perform each stage of the detector on the window */
					for(k=0; k<stages.size(); k++)
					{
						
						/*If one stage fails, the zone is rejected.*/
						if(!stages[k].pass(integral, squares, x, y, scale))
						{
							pass = false;
							break;
						}
					}
					/* If the window passed all stages, add it to the results. */
					if(pass)
					{
						printf("+");
						objects.push_back(Rectangle(x, y, w, h, 0));
					}
				}
			}
			printf("\n");
		}
		/* The size of detected objects is in a increase sequence */
		printf("Total found %d objects\n", objects.size());
		return merge(objects, min_neighbors);
	}
	
	/** Merge the raw detections resulting from the detection step 
	 * to avoid multiple detections of the same object.
	 * A threshold on the minimum numbers of rectangles
	 * that need to be merged for the resulting detection to be kept can be given,
	 * to lower the rate of false detections.
	 * Two rectangles need to be merged if they overlap enough.
	 * @param rects The raw detections returned by the detection algorithm.
	 * @param min_neighbors The minimum number of rectangles needed for the corresponding detection to be kept.
	 * @return The merged rectangular detections.
	 */
	vector<Rectangle> merge(vector<Rectangle> objs, unsigned int min_neighbors)
	{
		vector<Rectangle> ret;
		int *mark = new int[objs.size()];
		int nb_classes = 0;
		/* mark each rectangle with a class number */
		for(unsigned int i=0; i<objs.size(); i++)
		{
			bool found = false;
			for(unsigned int j=0; j<i; j++)
			{
				if(equals(objs[i], objs[j]))
				{
					found = true;
					mark[i] = mark[j];
				}
			}
			if(!found)
			{
				mark[i] = nb_classes;
				nb_classes++;
			}
		}
		unsigned int *neighbors = new unsigned int[nb_classes];
		Rectangle *rects = new Rectangle[nb_classes];
		for(int i=0; i<nb_classes; i++)
		{
			neighbors[i] = 0;
			rects[i].x = rects[i].y = rects[i].width = rects[i].height = 0;
		}
		/* calculate number of rects of each class */
		for(unsigned int i=0; i<objs.size(); i++)
		{
			neighbors[mark[i]]++;
			rects[mark[i]].x += objs[i].x;
			rects[mark[i]].y += objs[i].y;
			rects[mark[i]].width += objs[i].width;
			rects[mark[i]].height += objs[i].height;
		}
		for(int i=0; i<nb_classes; i++)
		{
			unsigned int n = neighbors[i];
			if(n >= min_neighbors)
			{
				Rectangle r;
				r.x = (rects[i].x*2 + n)/(2*n);
				r.y = (rects[i].y*2 + n)/(2*n);
				r.width = (rects[i].width*2 + n)/(2*n);
				r.height = (rects[i].height*2 + n)/(2*n);
				ret.push_back(r);
			}
		}
        delete []mark;
        delete []neighbors;
        delete []rects;
		return ret;
	}
	
	/** Returns true if two rectangles overlap */
	bool equals(Rectangle& r1, Rectangle& r2)
	{
		int distance = (int)(r1.width * 0.2);
		/* r1.x - distance <= r2.x <= r1.x + distance
		 * r1.y - distance <= r2.y <= r1.y + distance
		 */
		if(	r2.x <= r1.x + distance &&
			r2.x >= r1.x - distance &&
			r2.y <= r1.y + distance &&
			r2.y >= r1.y - distance &&
			r2.width <= (int)(r1.width * 1.2) &&
			r1.width <= (int)(r2.width * 1.2) )
			return true;
		if(	r1.x>=r2.x && r1.x+r1.width<=r2.x+r2.width &&
			r1.y>=r2.y && r1.y+r1.height<=r2.y+r2.height )
			return true;
		return false;
	}
}Detector;

#endif

