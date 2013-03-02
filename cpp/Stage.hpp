#ifndef __STAGE_H__
#define __STAGE_H__
#include <vector>
#include <math.h>
#include "Tree.hpp"
using namespace std;

/**
 * A stage of the detector. Each stage consists of several trees and a threshold.
 * When using the detector on a window, each tree returns a value.
 * If the sum of these values exceeds the threshold, the stage succeeds
 * else it fails (and the window is not the object looked for).
 */
typedef struct Stage
{
	vector<Tree> trees;
	double threshold;
	Stage()
	{}
	Stage(double threshold)
	{
		this->threshold = threshold;
	}
	void addTree(Tree t)
	{
		this->trees.push_back(t);
	}
	
	bool pass(Image& grayImage, Image& squares, int i, int j, double scale)
	{
		double sum = 0;
		/* Compute the sum of values returned by each tree of the stage. */
		for(unsigned int i=0; i<trees.size(); i++)
		{
			sum += trees[i].getVal(grayImage, squares,i, j, scale);
		}
		/* The stage succeeds if the sum exceeds the stage threshold */
		return sum > threshold;
	}

}Stage;

#endif

