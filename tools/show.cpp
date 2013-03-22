#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "../c/mshcd.hpp"
using namespace cv;

u32 GetHaarCascade(const char* filename, vector<Stage>& Stages);

static HaarCascade haarcascade;

int main(int argc, const char* argv[])
{
	const double scale = 30;
	const unsigned int ext = 20;
	unsigned int i, j, k, size1, size2;

	if(argc<2)
	{
		printf("Usage: %s filename\n", argv[0]);
		return -1;
	}

	Scalar black(0, 0, 0, 0);
	Scalar gray(190, 190, 190, 0);
	Scalar white(255, 255, 255, 0);
	size1 = size2 = GetHaarCascade(argv[1], haarcascade.stages);
	Size size(size1*scale+ext, size2*scale+ext);
	for(i=0; i<haarcascade.stages.size(); i++)
	{
		Mat image(size, CV_8UC(3), gray);
		Stage &stage = haarcascade.stages[i];
		printf("Stage %d trees %d\n", i+1, stage.trees.size());
		for(j=0; j<stage.trees.size(); j++)
		{
			Tree &tree = stage.trees[j];
			Scalar color(random()%255, random()%255, random()%255, 0);
			int bold = random()%2+1;
			for(k=0; k<tree.nb_rects; k++)
			{
				Rect r;
				r.x = tree.rects[k].x*scale + ext;
				r.y = tree.rects[k].y*scale + ext;
				r.width = tree.rects[k].width*scale;
				r.height = tree.rects[k].height*scale;	
				rectangle(image, r, color, bold, CV_AA, 0);
				if(k==0)
					circle(image, cv::Point(r.x, r.y), 6.5, white, -1, CV_AA, 0);
				else
					circle(image, cv::Point(r.x, r.y), 3.3, black, -1, CV_AA, 0);
			}
			waitKey();
			imshow("rectangles", image);
		}
	}
	return 0;
}

