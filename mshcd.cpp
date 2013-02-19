#include <stdio.h>
#include <math.h>
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
	double ScaleUpdate;
	vector<Stage> stages;
}HaarCascade;
static HaarCascade haarcascade;

typedef struct Image
{
	unsigned int width, height;
	unsigned char* data;
	unsigned long* idata;
}Image;
static Image image;

void GetHaarCasade(const char* filename);
void GetIntergralImages(const char* imagefile);
void HaarCasadeObjectDetection();

void mshcd(const char* imagefile, const char* haarcasadefile)
{
	haarcascade.size1 = 20;
	haarcascade.size2 = 20;
	haarcascade.ScaleUpdate = 1/1.2;
	GetHaarCasade(haarcasadefile);
	GetIntergralImages(imagefile);
	HaarCasadeObjectDetection();
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
		assert(stage.trees.size() == trees);
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
	unsigned long size;
	printf("%s()\n", __FUNCTION__);
	
	fin = fopen(imagefile, "rb");
	fscanf(fin, "%d %d\n", &image.width, &image.height);
	printf("%d X %d\n", image.width, image.height);
	size = image.width*image.height;
	image.data = (unsigned char*)malloc(size*sizeof(unsigned char));
	fread(image.data, image.width*image.height, 1, fin);
	fclose(fin);
	
	image.idata = (unsigned long*)malloc(size*sizeof(unsigned long));
	memset(image.idata, 0, size*sizeof(unsigned long));
	for(i=0; i<image.width; i++)
		for(j=0; j<image.height; j++)
		{
			/*for(m=0; m<i; m++)
				for(n=0; n<j; n++)
				{
					*(image.idata+i*image.width+j) += \
					*(image.data+m*image.width+n);
				}*/
			m = i, n = j;
			*(image.idata+i*image.width+j) = \
					*(image.data+m*image.width+n);
		}
}

void HaarCasadeObjectDetection()
{
	int Scale, StartScale, ScaleWidth, ScaleHeight;
	long i, itt;
	printf("%s()\n", __FUNCTION__);
	ScaleWidth = image.width/haarcascade.size1;
	ScaleHeight = image.height/haarcascade.size2;
	if(ScaleHeight < ScaleWidth)
		StartScale =  ScaleHeight; 
	else
		StartScale = ScaleWidth;
	printf("StartScale = %d\n", StartScale);
	itt = ceil(log(1.0/StartScale)/log(haarcascade.ScaleUpdate));
	printf("itt = %lf / %lf = %ld\n", \
			log(1.0/StartScale), log(haarcascade.ScaleUpdate), itt);
	for(i=0; i<itt; i++)
	{
		unsigned int w, h, step;
		Scale = StartScale*pow(haarcascade.ScaleUpdate,(i-1));
		step = (Scale>2)?Scale:2;
		printf("itt %ld scale %d step %d\n", i+1, Scale, step);
		w = floor(haarcascade.size1*Scale);
		h = floor(haarcascade.size2*Scale);
		// Make vectors with all search image coordinates 
		/*[x,y]=ndgrid(0:step:
		(IntegralImages.width-w-1),0:
		step:
		(IntegralImages.height-h-1));
		x=x(:); y=y(:);
		*/
		//OneScaleObjectDetection(x, y, Scale, w, h)
	}
}

unsigned long GetSumRect(unsigned int x, unsigned int y,
                         unsigned int w, unsigned int h)
{
	return	*( image.idata+(x+w)*image.width+(y+h) )
		-	*( image.idata+(x+0)*image.width+(y+h) )
		-	*( image.idata+(x+w)*image.width+(y+0) )
		+	*( image.idata+(x+0)*image.width+(y+0) );
}
/*
OneScaleObjectDetection(x, y, Scale, w, h)
{
	int i_stage, i_tree;
	double InverseArea;
	InverseArea = 1 / (w*h);
	mean = GetSumRect*InverseArea;
	Variance = GetSumRect(IntegralImages.ii2,x,y,w,h)*InverseArea - (mean.^2);

	for(i_stage=0; i_stage<haarcascade.stages.size(); i_stage++)
	{
		long StageSum = 0;
		for(i_tree=0; i_tree<c.trees.size(); i_tree++)
		{
			TreeSum = TreeObjectDection();
			StageSum += TreeSum;
		}
		check = StageSum < haarcascade.stages[i].threshold;
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
*/

int main()
{
	mshcd("./tools/gray_img.raw", "haar.txt");
	return 0;
}

