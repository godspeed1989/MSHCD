#include <stdio.h>
#include <assert.h>
#include <vector>
using namespace std;

typedef struct Tree
{
	int rect1[5], rect2[5];
	int tilted;
	double threshold;
	double left_val, right_val;
}Tree;

typedef struct Stage
{
	double threshold;
	vector<Tree> trees;
}Stage;

static Tree tree;
static Stage stage;
static vector<Stage> HaarCascade;

void GetHaarCasade(const char* filename);

void mshcd(const char* imagefile, const char* haarcasadefile)
{
	GetHaarCasade(haarcasadefile);
	//GetIntergralImages(imagefile);
	//HaarCasadeObjectDetection(image, haarcasade);
}

void GetHaarCasade(const char* filename)
{
	FILE *fin;
	int i, j, n, n_stages, n_trees;
	int stages, trees, value;
	printf("%s()\n", __FUNCTION__);
	fin = fopen(filename, "r");
	assert(fin);
	fscanf(fin, "%d", &n_stages);
	printf("Total %d stages\n", n_stages);
	n = 1;
	while(!feof(fin) && n <= n_stages)
	{
		fscanf(fin, "%d", &n_trees); // num of trees per stage
		printf("Stage %d num_tree %d ", n++, n_trees);
		trees = 1;
		while(trees < n_trees && !feof(fin))  // get each tree
		{
			for(i=0; i<6; i++)
			{
				fscanf(fin, "%d%d%d", &stages, &trees, &value);
				switch(i)
				{
					case 0:
						fscanf(fin, "%d", &value);
						for(j=0; j<5; j++) fscanf(fin, "%d", &tree.rect1[j]);
						break;
					case 1:
						fscanf(fin, "%d", &value);
						for(j=0; j<5; j++) fscanf(fin, "%d", &tree.rect2[j]);
						break;
					case 2: fscanf(fin, "%d", &tree.tilted);
						break;
					case 3: fscanf(fin, "%lf", &tree.threshold);
						break;
					case 4: fscanf(fin, "%lf", &tree.left_val);
						break;
					case 5: fscanf(fin, "%lf", &tree.right_val);
						break;
				}	
			}
			stage.trees.push_back(tree);
		}
		fscanf(fin, "%d %lf", &stages, &stage.threshold); // get threshold of stage
		printf("threshold %lf\n", stage.threshold);
	}
	/*stages = ;
	for(i...stages)
	{
		stages2 = 
		for(j...stages[i].trees)
		{
			for(k...stages[i].trees[j].value)
			{
				
				stages2[i].trees[j].value[k] = 
				{a.threshold a.left_val a.right_val 
				a.left_node a.right_node a.rects1 a.rects2 a.rects3 
				a.tilted}
			}
		}
	}*/
}
/*
GetIntergralImages(imagefile)
{
	resize(image);
	convert_gray(imagefile);
	make integral image;	
}

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
	mshcd("", "haar.txt");
	return 0;
}

