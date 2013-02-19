#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
using namespace std;

typedef struct Rect{
	int r[5];
}Rect;
typedef struct Tree
{
	vector<Rect> rects;
	int rect2[5];
	int tilted;
	double threshold;
	double left_val, right_val;
}Tree;
typedef struct Stage
{
	double threshold;
	vector<Tree> trees;
}Stage;
typedef struct HaarCascade
{
	int size1, size2;
	vector<Stage> stages;
}HaarCascade;
static HaarCascade haarcascade;

typedef struct Image
{
	unsigned int rows, cols;
	unsigned char* data;
	unsigned long* idata;
}Image;
static Image image;

void GetHaarCasade(const char* filename);
void GetIntergralImages(const char* imagefile);

void mshcd(const char* imagefile, const char* haarcasadefile)
{
	GetHaarCasade(haarcasadefile);
	//GetIntergralImages(imagefile);
	//HaarCasadeObjectDetection(image, haarcasade);
}

void GetHaarCasade(const char* filename)
{
	FILE *fin;
	int i, j, t, n, n_stages, n_trees;
	int stages, trees, value, flag;
	printf("%s()\n", __FUNCTION__);
	fin = fopen(filename, "r");
	assert(fin);
	fscanf(fin, "%d", &n_stages);
	printf("Total %d stages\n", n_stages);
	n = 1;
	while(!feof(fin) && n <= n_stages)
	{
		Stage stage;
		fscanf(fin, "%d", &n_trees); // num of trees per stage
		printf("Stage %d num_tree %d ", n, n_trees);
		t = 1;
		while(t <= n_trees && !feof(fin))  // get each tree
		{
			Tree tree;
			i = 0;
			flag = 0;
			trees = t;
			while(t==trees && i<7)
			{
				fscanf(fin, "%d%d%d", &stages, &trees, &value);
				assert(stages == n);
				assert(value == 1);
				if(i < 2)
				{
					Rect rect;
					fscanf(fin, "%d", &value);
					for(j=0; j<5; j++) fscanf(fin, "%d", &rect.r[j]);
					tree.rects.push_back(rect);
				}
				else if(i == 2)
				{
					fscanf(fin, "%d", &value);
					if(value == 0)
						tree.tilted = 0;
					else if(value == 3) // exist 3rd rect
					{
						flag = 1;
						Rect rect;
						for(j=0; j<5; j++) fscanf(fin, "%d", &rect.r[j]);
						tree.rects.push_back(rect);
					}
					else
						assert(0);
				}
				else
				{
					if(flag) i--;  // exist 3rd rect
					switch(i)
					{
						case 2: fscanf(fin, "%d", &tree.tilted);     break;
						case 3: fscanf(fin, "%lf", &tree.threshold); break;
						case 4: fscanf(fin, "%lf", &tree.left_val);  break;
						case 5: fscanf(fin, "%lf", &tree.right_val); break;
					}
					if(flag) i++;
					if(!flag && i==5) i++;
				}
				i++;
			}
			t++;
			stage.trees.push_back(tree);
		}
		n++;
		printf("%d  ", trees);
		fscanf(fin, "%d %lf", &stages, &stage.threshold); // get threshold of stage
		printf("threshold %lf\n", stage.threshold);
		haarcascade.stages.push_back(stage);
	}
	
	for(i=0; i<haarcascade.stages.size(); i++)
	{
		printf("%d\t", i+1);
		printf("%d\n", haarcascade.stages[i].trees.size());
	}
}

void GetIntergralImages(const char* imagefile)
{
	FILE *fin;
	int i, j, m, n;
	printf("%s()\n", __FUNCTION__);
	
	fin = fopen(imagefile, "rb");
	fscanf(fin, "%d %d\n", &image.rows, &image.cols);
	printf("%d X %d\n", image.rows, image.cols);
	image.data = (unsigned char*)malloc(image.rows*image.cols*1);
	fread(image.data, image.rows*image.cols, 1, fin);
	fclose(fin);
	
	image.idata = (unsigned long*)malloc(image.rows*image.cols*sizeof(unsigned long));
	memset(image.idata, 0, image.rows*image.cols*sizeof(unsigned long));
	for(i=0; i<image.rows; i++)
		for(j=0; j<image.cols; j++)
		{
			for(m=0; m<i; m++)
				for(n=0; n<j; n++)
				{
					*(image.idata+i*image.rows+j) += \
					*(image.data+m*image.rows+n);
				}
		}
	/*resize(image);
	convert_gray(imagefile);
	make integral image;*/
}
/*
HaarCasadeObjectDetection(image, haarcasade)
{
	get coarsest image scale
	for(i...itt)
	{
		OneScaleObjectDetection();
	}
}

OneScaleObjectDetection()
{
	//loop through all claasifer stages
	for(i_stage...stages)
	{
		// loop through a classifier tree
		for(i_tree...trees)
		{
			TreeSum = TreeObjectDetection();
			StageSum += TreeSum;
		}
	}
}

TreeObjectDetection()
{
	//Calculate the haar-feature response
	for(i_rectangle=1..3)
	{
		r_sum = GetSumRect();
		Rectangle_sum += r_sum;
	}
	Tree_sum = TreeObjectDetection
}

GetSumRect()
{
	IIWidth=size(IntegralImage,1);
	PixSum  = IntegralImage((x+Width)*IIWidth + y + Height+1) \
			+ IntegralImage(x*IIWidth+y+1) \
			- IntegralImage((x+Width)*IIWidth+y+1) \
			- IntegralImage(x*IIWidth+y+Height+1);
}
*/

int main()
{
	mshcd("./tools/gray_img.raw", "haar.txt");
	return 0;
}

