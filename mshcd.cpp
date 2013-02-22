#include "mshcd.hpp"

static HaarCascade haarcascade;
static Image image;
static vector<Rect> objects;

void GetHaarCascade(const char* filename);
void GetIntergralImages(const char* imagefile);
void HaarCasadeObjectDetection();
void OneScaleObjectDetection(vector<Point> points, double Scale,
                             unsigned int w, unsigned int h);
double TreeObjectDetection(Tree& tree, double Scale, Point& point,
                           vector<double>& StandardDeviation, 
                           double InverseArea);

void mshcd(const char* imagefile, const char* haarcasadefile)
{
	haarcascade.size1 = 20;
	haarcascade.size2 = 20;
	haarcascade.ScaleUpdate = 1/1.2;
	GetHaarCascade(haarcasadefile); // get classifer from file
	GetIntergralImages(imagefile);  // calculate intergral image
	HaarCasadeObjectDetection();    // start detection
}
/**
 * get Haar Cascade classifier from file
 */
void GetHaarCascade(const char* filename)
{
	FILE *fin;
	unsigned int i, t, s, n_stages, n_trees;
	unsigned int stages, trees, value, flag;
	unsigned long total_features;
	PRINT_FUNCTION_INFO();
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
		while(t <= n_trees && !feof(fin))  // get each tree
		{
			Tree tree;
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
					fscanf(fin, "%d", &rect.x);
					fscanf(fin, "%d", &rect.y);
					fscanf(fin, "%d", &rect.width);
					fscanf(fin, "%d", &rect.height);
					fscanf(fin, "%d", &rect.weight);
					tree.rects[i] = rect;
				}
				else if(i == 2)
				{
					Rect rect;
					fscanf(fin, "%d", &value);
					if(value == 0)
					{
						tree.tilted = 0;
						memset(&rect, 0, sizeof(Rect));
						tree.rects[i] = rect;
					}
					else if(value == 3) // exist 3rd rect
					{
						flag = 1;
						fscanf(fin, "%d", &rect.x);
						fscanf(fin, "%d", &rect.y);
						fscanf(fin, "%d", &rect.width);
						fscanf(fin, "%d", &rect.height);
						fscanf(fin, "%d", &rect.weight);
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
		s++;
		assert(stage.trees.size() == trees);
		fscanf(fin, "%d %lf", &stages, &stage.threshold); // get threshold of stage
		printf("threshold %lf\n", stage.threshold);
		haarcascade.stages.push_back(stage);
	}
	total_features = 0;
	for(i=0; i<haarcascade.stages.size(); i++)
	{
		printf("%d\t", i+1);
		printf("%d\n", haarcascade.stages[i].trees.size());
		total_features += haarcascade.stages[i].trees.size();
	}
	printf("Total features %ld\n", total_features);
}

/**
 * calculate intergral image from original gray-level image
 */
void GetIntergralImages(const char* imagefile)
{
	FILE *fin;
	unsigned int i, j, m, n;
	unsigned long size;
	PRINT_FUNCTION_INFO();
	
	fin = fopen(imagefile, "rb");
	assert(fin);
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
 * Dectect object use Multi-scale Haar cascade classifier
 */
void HaarCasadeObjectDetection()
{
	double Scale, StartScale, ScaleWidth, ScaleHeight;
	unsigned int i, itt, x, y;
	PRINT_FUNCTION_INFO();
	// get start scale
	ScaleWidth = image.width/haarcascade.size1;
	ScaleHeight = image.height/haarcascade.size2;
	if(ScaleHeight < ScaleWidth)
		StartScale = ScaleHeight; 
	else
		StartScale = ScaleWidth;
	printf("StartScale = %lf\n", StartScale);
	itt = ceil(log(1.0/StartScale)/log(haarcascade.ScaleUpdate));
	printf("itt = %lf / %lf = %d\n", \
			log(1.0/StartScale), log(haarcascade.ScaleUpdate), itt);
	// iteration through the each search
	for(i=0; i<itt; i++)
	{
		unsigned int width, height, step;
		Scale = StartScale*pow(haarcascade.ScaleUpdate,(i-1));
		step = (Scale>2.0)?Scale:2.0;
		printf("itt %d scale %lf step %d ", i+1, Scale, step);
		width = floor(haarcascade.size1*Scale);
		height = floor(haarcascade.size2*Scale);
		printf("width %d height %d\n", width, height);
		// Make vectors with all search image coordinates 
		Point point;
		vector<Point> points;
		//0:step:(IntegralImages.width-width-1),
		//0:step:(IntegralImages.height-height-1)
		for(x=0; x<image.width; x+=step)
		{
			for(y=0; y<image.height; y+=step)
			{
				point.x = x, point.y = y;
				points.push_back(point);
			}
		}
		OneScaleObjectDetection(points, Scale, width, height);
	}
}

unsigned long GetSumRect(int type,
                         unsigned int x, unsigned int y,
                         unsigned int w, unsigned int h)
{
	assert(x+w<image.width && y+h<image.height);
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
 * detect the object at a fixed scale, fixed width, fixed height
 */
void OneScaleObjectDetection(vector<Point> points, double Scale,
                             unsigned int width, unsigned int height)
{
	unsigned int i, i_stage, i_tree;
	vector<double> mean;
	vector<double> Variance;
	vector<double> StandardDeviation;
	double InverseArea;
	InverseArea = 1 / (width*height);
	// calculate the mean and gray-level varianceiance 
	// of every search window
	for(i=0; i<points.size(); i++)
	{
		mean.push_back( 
			GetSumRect(II1, points[i].x, points[i].y, width, height) \
			* InverseArea
		);
		Variance.push_back(
			GetSumRect(II2, points[i].x, points[i].y, width, height) \
			* InverseArea - pow(*mean.end(), 2)
		);
	}
	// Convert the Varianceiation to Standard Deviation
	for(i=0; i<Variance.size(); i++)
	{
		if(Variance[i]<1.0) Variance[i] = 1.0;
		StandardDeviation.push_back( sqrt(Variance[i]) );
	}
	// If a coordinate doesn't pass the classifier threshold
	// it is removed, otherwise it goes into the next classifier
	for(i=0; i<points.size(); i++)
	{
		for(i_stage=0; i_stage<haarcascade.stages.size(); i_stage++)
		{
			Stage &stage = haarcascade.stages[i_stage];
			double StageSum = 0.0;
			for(i_tree=0; i_tree<stage.trees.size(); i_tree++)
			{
				Tree &tree = stage.trees[i];
				double TreeSum;
				TreeSum = TreeObjectDetection(tree, Scale,
					points[i], StandardDeviation, InverseArea);
				StageSum += TreeSum;
			}
			// If the StageSum of a coordinate is lower than
			// the treshold it is removed, otherwise it goes into the next stage
			if(StageSum < stage.threshold)
			{
				break; // to the next point
			}
			else
			{
				continue; // to the next stage
			}
		}
		// pass all the stages
		if(i_stage == haarcascade.stages.size())
		{
			Rect rect;
			rect.x = points[i].x;
			rect.y = points[i].y;
			rect.width = width;
			rect.height = height;
			rect.weight = -1;
			objects.push_back(rect);
		}
	}
}

/**
 * get the current haar-classifier's value (left/right)
 */
double TreeObjectDetection(Tree& tree, double Scale, Point& point,
                           vector<double>& StandardDeviation, 
                           double InverseArea)
{
	//Calculate the haar-feature response
	unsigned int i, i_rect, check;
	double Rectangle_sum, tree_threshold;
	Rectangle_sum = tree_threshold = 0.0;
	for(i_rect=0; i_rect<3; i_rect++)
	{
		unsigned long r_sum = 0;
		Rect &rect = tree.rects[i_rect];
		unsigned int RectX = rect.x * Scale + point.x;
		unsigned int RectY = rect.y * Scale + point.y;
		unsigned int RectWidth = rect.width * Scale;
		unsigned int RectHeight = rect.height * Scale;
		r_sum = GetSumRect(II1, RectX, RectY, RectWidth, RectHeight) \
					* rect.weight;
		Rectangle_sum += r_sum;
	}
	Rectangle_sum *= InverseArea;
	for(i=0; i<StandardDeviation.size(); i++)
		tree_threshold += tree.threshold*StandardDeviation[i];
	check = Rectangle_sum >= tree_threshold * 1.1;
	/*
	Leaf= Tree(Node+1,:);
	LeafTreshold=Leaf(:,1);
	check = Rectangle_sum >= tree.threshold.*StandardDeviation;
	Node(check) = RightNode(check);
	Node(~check) = LeftNode(~check);
	Tree_sum(check) = RightValue(check);
	Tree_sum(~check) = LeftValue(~check);
	*/
	if(check)
		return tree.right_val;
	else
		return tree.left_val;
}

int main()
{
	mshcd("./tools/gray_img.raw", "haar.txt");
	return 0;
}
