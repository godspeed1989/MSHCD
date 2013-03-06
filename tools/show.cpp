#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "../mshcd.hpp"
using namespace cv;

void GetHaarCascade(const char* filename, vector<Stage>& Stages);

static HaarCascade haarcascade;

int main(int argc, const char* argv[])
{
	const int _size = 20;
	const double scale = 35;
	unsigned int i, j, k;
	Size size(_size*scale, _size*scale);
	Scalar white(255, 255, 255, 0);
	Scalar black(0, 0, 0, 0);
	
	GetHaarCascade("../haar.txt", haarcascade.stages);
	for(i=0; i<haarcascade.stages.size(); i++)
	{
		Mat image(size, CV_32F, black);
		Stage &stage = haarcascade.stages[i];
		printf("Stage %d trees %d\n", i+1, stage.trees.size());
		for(j=0; j<stage.trees.size(); j++)
		{
			Tree &tree = stage.trees[j];
			for(k=0; k<3; k++)
			{
				Rect r;
				r.x = tree.rects[k].x*scale;
				r.y = tree.rects[k].y*scale;
				r.width = tree.rects[k].width*scale;
				r.height = tree.rects[k].height*scale;
				rectangle(image, r, white, 1.5, 8, 0);
			}
		}
		imshow("rectangles", image);
		waitKey();
	}
	return 0;
}

