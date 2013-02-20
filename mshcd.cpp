#include "mshcd.hpp"

static HaarCascade haarcascade;
static Image image;

void GetHaarCasade(const char* filename);
void GetIntergralImages(const char* imagefile);
void HaarCasadeObjectDetection();
void OneScaleObjectDetection(vector<Point> points, double Scale,
                             unsigned int w, unsigned int h);

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
					tree.rects[i] = rect;
				}
				else if(i == 2)
				{
					Rect rect;
					fscanf(fin, "%d", &value);
					if(value == 0)
					{
						tree.tilted = 0;
						for(j=0; j<5; j++) rect.r[j] = 0;
						tree.rects[i] = rect;
					}
					else if(value == 3) // exist 3rd rect
					{
						flag = 1;
						for(j=0; j<5; j++) fscanf(fin, "%d", &rect.r[j]);
						tree.rects[i] = rect;
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
	
	image.idata1 = (unsigned long*)malloc(size*sizeof(unsigned long));
	memset(image.idata1, 0, size*sizeof(unsigned long));
	image.idata2 = (unsigned long*)malloc(size*sizeof(unsigned long));
	memset(image.idata2, 0, size*sizeof(unsigned long));
	for(i=0; i<image.width; i++)
		for(j=0; j<image.height; j++)
		{
			/*for(m=0; m<i; m++)
				for(n=0; n<j; n++)
				{
					*(image.idata1+i*image.width+j) += \
					*(image.data+m*image.width+n);
					*(image.idata2+i*image.width+j) += \
					pow(*(image.data+m*image.width+n), 2);
				}*/
			m = i, n = j;
			*(image.idata1+i*image.width+j) = \
					*(image.data+m*image.width+n);
			*(image.idata2+i*image.width+j) = \
					pow(*(image.data+m*image.width+n), 2);
		}
}

/**
 * Dectect object
 */
void HaarCasadeObjectDetection()
{
	double Scale, StartScale, ScaleWidth, ScaleHeight;
	long i, itt, x, y;
	printf("%s()\n", __FUNCTION__);
	ScaleWidth = image.width/haarcascade.size1;
	ScaleHeight = image.height/haarcascade.size2;
	if(ScaleHeight < ScaleWidth)
		StartScale =  ScaleHeight; 
	else
		StartScale = ScaleWidth;
	printf("StartScale = %lf\n", StartScale);
	itt = ceil(log(1.0/StartScale)/log(haarcascade.ScaleUpdate));
	printf("itt = %lf / %lf = %ld\n", \
			log(1.0/StartScale), log(haarcascade.ScaleUpdate), itt);
	// iteration through the each search
	for(i=0; i<itt; i++)
	{
		unsigned int w, h, step;
		Scale = StartScale*pow(haarcascade.ScaleUpdate,(i-1));
		step = (Scale>2.0)?Scale:2.0;
		printf("itt %ld scale %lf step %d\n", i+1, Scale, step);
		w = floor(haarcascade.size1*Scale);
		h = floor(haarcascade.size2*Scale);
		// Make vectors with all search image coordinates 
		Point point;
		vector<Point> points;
		for(x=0; x<image.width; x+=step)
		{
			for(y=0; y<image.height; y+=step)
			{
				point.x = x, point.y = y;
				points.push_back(point);
			}
		}
		/*vector<Point> points =ndgrid(
				0:step:(IntegralImages.width-w-1),
				0:step:(IntegralImages.height-h-1)
			);*/
		
		OneScaleObjectDetection(points, Scale, w, h);
	}
}

unsigned long GetSumRect(int type,
                         unsigned int x, unsigned int y,
                         unsigned int w, unsigned int h)
{
	if(type == II1)
		return	*( image.idata1+(x+w)*image.width+(y+h) )
			-	*( image.idata1+(x+0)*image.width+(y+h) )
			-	*( image.idata1+(x+w)*image.width+(y+0) )
			+	*( image.idata1+(x+0)*image.width+(y+0) );
	if(type == II2)
		return	*( image.idata2+(x+w)*image.width+(y+h) )
			-	*( image.idata2+(x+0)*image.width+(y+h) )
			-	*( image.idata2+(x+w)*image.width+(y+0) )
			+	*( image.idata2+(x+0)*image.width+(y+0) );
	assert(0);
	return 0;
}

/**
 * detect the object at a fixed scale
 */
void OneScaleObjectDetection(vector<Point> points, double Scale,
                             unsigned int w, unsigned int h)
{
	int i, i_stage, i_tree;
	vector<double> mean;
	vector<double> Variance;
	vector<double> StandardDeviation;
	double InverseArea;
	InverseArea = 1 / (w*h);
	// calculate the mean and gray-level varianceiance 
	// of every search window
	for(i=0; i<points.size(); i++)
	{
		double m, v;
		m = GetSumRect(II1,points[i].x,points[i].y,w,h)*InverseArea;
		v = GetSumRect(II2,points[i].x,points[i].y,w,h)*InverseArea-pow(m, 2);
		mean.push_back(m);
		Variance.push_back(v);
	}
	// Convert the Varianceiation to Standard Deviation of every window
	for(i=0; i<Variance.size(); i++)
	{
		if(Variance[i]<1.0) Variance[i] = 1.0;
		StandardDeviation.push_back(sqrt(Variance[i]));
	}
	vector<unsigned long> TreeSum;
	// If a coordinate doesn't pass the classifier threshold
	// it is removed, otherwise it goes into the next classifier
	for(i=0; i<points.size(); i++)
	{
		/*for(i_stage=0; i_stage<haarcascade.stages.size(); i_stage++)
		{
			Stage &stage = haarcascade.stages[i_stage];
			long StageSum = 0;
			for(i_tree=0; i_tree<stage.trees.size(); i_tree++)
			{
				Tree &tree = stage.trees[i];
				TreeSum = TreeObjectDection(tree, Scale,
					points[i].x, points[i].y,
					StandardDeviation, InverseArea);
				StageSum += TreeSum;
			}
			// If the StageSum of a coordinate is lower than
			// the treshold it is removed, otherwise it goes into the next stage
			check = (StageSum < stage.threshold);
			if(check)
			{
				points.remove(i);
				break;
			}
		}*/
	}
}
/*
unsigned long TreeObjectDetection(Tree& tree, double Scale,
                                  unsigned int x, unsigned int y,
                                  StandardDeviation, double InverseArea)
{
	//Calculate the haar-feature response
	int i_rect;
	Rectangle_sum = zeros(size(x));
	for(i_rect=0; i_rect<3; i_rect++)
	{
		unsigned long r_sum = 0;
		Rect &rect = tree.rects[i_rect];
		unsigned int RectX = rect.r[0]*Scale+x;
		unsigned int RectY = rect.r[1]*Scale+y;
		unsigned int RectWidth = rect.r[2]*Scale;
		unsigned int RectHeight = rect.r[3]*Scale;
		unsigned int RectWeight = rect.r[4];
		r_sum = GetSumRect(II1, RectX, RectY,
						RectWidth, RectHeight)*RectWeight;
		Rectangle_sum += r_sum;
	}
	Rectangle_sum *= InverseArea;
	
	check = Rectangle_sum >= LeafTreshold(:).*StandardDeviation;
	Node(check) = RightNode(check);
	Node(~check) = LeftNode(~check);
	Tree_sum(check) = RightValue(check);
	Tree_sum(~check) = LeftValue(~check);
	if(check)
		return tree.left_val;
	else
		return tree.right_val;
}
*/

int main()
{
	mshcd("./tools/gray_img.raw", "haar.txt");
	return 0;
}

