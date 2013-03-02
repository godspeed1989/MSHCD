#ifndef __DETECTOR_H__
#define __DETECTOR_H__

#include <vector>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "Stage.hpp"
#include "Point.hpp"
using namespace std;

typedef struct Detector
{
	Point size;
	vector<Stage> stages;
	
	Detector(const char* filename)
	{
		size.x = 20;
		size.y = 20;
		FILE *fin;
		unsigned int i, t, s, n_stages, n_trees;
		unsigned int stages, trees, value, flag;
		unsigned long total_features;
		fin = fopen(filename, "r");
		assert(fin);
		fscanf(fin, "%d", &n_stages);
		printf("Total %d stages\n", n_stages);
		s = 1;
		while(!feof(fin) && s <= n_stages) // get each stage
		{
			Stage stage;
			fscanf(fin, "%d", &n_trees); // num of trees per stage
			printf("Stage %d num_tree %d ", s, n_trees);
			t = 1;
			while(t <= n_trees && !feof(fin))  // get each tree's feature
			{
				Tree tree;
				Feature feature;
				i = 0;
				flag = 0;
				trees = t;
				while(t==trees && i<7)
				{
					fscanf(fin, "%d%d%d", &stages, &trees, &value);
					assert(stages == s);
					assert(value == 1);
					if(i < 2)
					{
						Rect rect;
						fscanf(fin, "%d", &value);
						assert(value == i+1);
						fscanf(fin, "%d", &rect.x1);
						fscanf(fin, "%d", &rect.x2);
						fscanf(fin, "%d", &rect.y1);
						fscanf(fin, "%d", &rect.y2);
						fscanf(fin, "%lf", &rect.weight);
						feature.rects[i] = rect;
					}
					else if(i == 2)
					{
						Rect rect;
						fscanf(fin, "%d", &value);
						if(value == 0)
						{
							feature.tilted = 0;
							memset(&rect, 0, sizeof(Rect));
							feature.rects[i] = rect;
						}
						else if(value == 3) // exist 3rd rect
						{
							flag = 1;
							fscanf(fin, "%d", &rect.x1);
							fscanf(fin, "%d", &rect.x2);
							fscanf(fin, "%d", &rect.y1);
							fscanf(fin, "%d", &rect.y2);
							fscanf(fin, "%lf", &rect.weight);
							feature.rects[i] = rect;
						}
						else
							assert(0);
					}
					else
					{
						if(flag) i--;  // exist 3rd rect
						switch(i)
						{
							case 2: fscanf(fin, "%d", &feature.tilted);     break;
							case 3: fscanf(fin, "%f", &feature.threshold); break;
							case 4: fscanf(fin, "%f", &feature.left_val);  break;
							case 5: fscanf(fin, "%f", &feature.right_val); break;
						}
						if(flag) i++;
						if(!flag && i==5) i++;
					}
					i++;
				}
				t++;
				tree.addFeature(feature);
				stage.addTree(tree);
			}
			s++;
			assert(stage.trees.size() == trees);
			fscanf(fin, "%d %f", &stages, &stage.threshold); // get threshold of stage
			printf("threshold %lf\n", stage.threshold);
			this->stages.push_back(stage);
		}
		total_features = 0;
		for(i=0; i<this->stages.size(); i++)
		{
			printf("%d\t", i+1);
			printf("%d\n", this->stages[i].trees.size());
			total_features += this->stages[i].trees.size();
		}
		printf("Total features %ld\n", total_features);
	}

	/** Returns the list of detected objects in an image applying the Viola-Jones algorithm.
	 * 
	 * The algorithm tests, from sliding windows on the image, of variable size, which regions should be considered as searched objects.
	 * Please see Wikipedia for a description of the algorithm.
	 * @param file The image file to scan.
	 * @param baseScale The initial ratio between the window size and the Haar classifier size (default 2).
	 * @param scale_inc The scale increment of the window size, at each step (default 1.25).
	 * @param increment The shift of the window at each sub-step, in terms of percentage of the window size.
	 * @return the list of rectangles containing searched objects, expressed in pixels.
	 */
	vector<Rectangle> getFaces(Image image, float baseScale,
		float scale_inc, float increment)
	{
		vector<Rectangle> ret;
		int width = image.getWidth();
		int height = image.getHeight();
		/* Compute the max scale of the detector, i.e. the size of the image divided by the size of the detector. */
		float maxScale = min((width+0.0f)/size.x, (height+0.0f)/size.y);
			
		/* Compute the grayscale image, the integral image and the squared integral image.*/
		Image grayImage(width, height);
		Image squares(width, height);
		for(int i=0; i<width; i++)
		{
			int col=0;
			int col2=0;
			for(int j=0; j<height; j++)
			{
				unsigned int value = image(i,j);
				grayImage(i,j) = (i>0?grayImage(i-1,j):0) + col + value;
				squares(i,j) = (i>0?squares(i-1,j):0) + col2 + value*value;
				col += value;
				col2 += value*value;
			}
		}
		
		/*Heart of the algorithm : detection */
		/*For each scale of the detection window */
		for(float scale=baseScale; scale<maxScale; scale*=scale_inc)
		{
			/*Compute the sliding step of the window*/
			int step = (int) (scale * size.x * increment);
			int w = (int) (scale * size.x);
			int h = (int) (scale * size.y);
			/*For each position of the window on the image, check whether the object is detected there.*/
			for(int i=0; i<width-w; i+=step)
			{
				for(int j=0; j<height-h; j+=step)
				{
					/* If Canny pruning is on, compute the edge density of the zone.
					 * If it is too low, the object should not be there so skip the region.*/
					bool pass = true;
					/* Perform each stage of the detector on the window */
					for(unsigned int k; k<stages.size(); k++)
					{
						/*If one stage fails, the zone is rejected.*/
						if(!stages[k].pass(grayImage, squares, i, j, scale))
						{
							pass = false;
							break;
						}
					}
					/* If the window passed all stages, add it to the results. */
					if(pass) ret.push_back(Rectangle(i, j, w, h));
				}
			}
		}
		return ret;
		//return merge(ret,min_neighbors);
	}
	
	/** Merge the raw detections resulting from the detection step to avoid multiple detections of the same object.
	 * A threshold on the minimum numbers of rectangles that need to be merged for the resulting detection to be kept can be given,
	 * to lower the rate of false detections.
	 * Two rectangles need to be merged if they overlap enough.
	 * @param rects The raw detections returned by the detection algorithm.
	 * @param min_neighbors The minimum number of rectangles needed for the corresponding detection to be kept.
	 * @return The merged rectangular detections.
	
	public List<java.awt.Rectangle> merge(List<java.awt.Rectangle> rects, int min_neighbors)
	{
		 List<java.awt.Rectangle> retour=new  LinkedList<java.awt.Rectangle>();
		int[] ret=new int[rects.size()];
		int nb_classes=0;
		for(int i=0;i<rects.size();i++)
		{
			boolean found=false;
			for(int j=0;j<i;j++)
			{
				if(equals(rects.get(j),rects.get(i)))
				{
					found=true;
					ret[i]=ret[j];
				}
			}
			if(!found)
			{
				ret[i]=nb_classes;
				nb_classes++;
			}
		}
		int[] neighbors=new int[nb_classes];
		Rectangle[] rect=new Rectangle[nb_classes];
		for(int i=0;i<nb_classes;i++)
		{
			neighbors[i]=0;
			rect[i]=new Rectangle(0,0,0,0);
		}
		for(int i=0;i<rects.size();i++)
		{
			neighbors[ret[i]]++;
			rect[ret[i]].x+=rects.get(i).x;
			rect[ret[i]].y+=rects.get(i).y;
			rect[ret[i]].height+=rects.get(i).height;
			rect[ret[i]].width+=rects.get(i).width;
		}
		for(int i = 0; i < nb_classes; i++ )
        {
            int n = neighbors[i];
            if( n >= min_neighbors)
            {
            	Rectangle r=new Rectangle(0,0,0,0);
                r.x = (rect[i].x*2 + n)/(2*n);
                r.y = (rect[i].y*2 + n)/(2*n);
                r.width = (rect[i].width*2 + n)/(2*n);
                r.height = (rect[i].height*2 + n)/(2*n);
                retour.add(r);
            }
        }
		return retour;
	} */
	
	/** Returns true if two rectangles overlap and should be merged.
	public boolean equals(Rectangle r1, Rectangle r2)
	{
		int distance = (int)(r1.width*0.2);

		if(r2.x <= r1.x + distance &&
	           r2.x >= r1.x - distance &&
	           r2.y <= r1.y + distance &&
	           r2.y >= r1.y - distance &&
	           r2.width <= (int)( r1.width * 1.2 ) &&
	           (int)( r2.width * 1.2 ) >= r1.width) return true;
		if(r1.x>=r2.x&&r1.x+r1.width<=r2.x+r2.width&&r1.y>=r2.y&&r1.y+r1.height<=r2.y+r2.height) return true;
		return false;
	}*/
}Detector;

#endif

