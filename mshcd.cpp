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
                           unsigned int width, unsigned int height);
void ShowDetectionResult();

void mshcd(const char* imagefile, const char* haarcasadefile)
{
	haarcascade.size1 = 20;
	haarcascade.size2 = 20;
	haarcascade.ScaleUpdate = 1.0/1.2;
	GetHaarCascade(haarcasadefile); // get classifer from file
	GetIntergralImages(imagefile);  // calculate intergral image
	HaarCasadeObjectDetection();    // start detection
	ShowDetectionResult();          // show detection result
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
					fscanf(fin, "%lf", &rect.weight);
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
						fscanf(fin, "%lf", &rect.weight);
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
	PRINT_FUNCTION_END_INFO();
}

/**
 * calculate intergral image from original gray-level image
 */
void GetIntergralImages(const char* imagefile)
{
	FILE *fin;
	unsigned int i, j;
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
	
	image.idata1 = (double*)malloc(size*sizeof(double));
	memset(image.idata1, 0, size*sizeof(double));
	image.idata2 = (double*)malloc(size*sizeof(double));
	memset(image.idata2, 0, size*sizeof(double));
	for(i=0; i<image.height; i++)
	{
		double row = 0.0;
		double row2 = 0.0;
		for(j=0; j<image.width; j++)
		{
			unsigned char value = *(image.data + i*image.width + j);	
			if(i>0)
			{
				*(image.idata1+i*image.width+j) += \
					*(image.idata1+(i-1)*image.width+j);
				*(image.idata2+i*image.width+j) += \
					*(image.idata2+(i-1)*image.width+j);
			}
			row += value;
			row2 += value*value;
			*(image.idata1+i*image.width+j) += row;
			*(image.idata2+i*image.width+j) += row2;
		}
	}
	/*for(i=0; i<image.height; i++)
	{
		for(j=0; j<image.width; j++)
			printf("%lf ", *(image.idata2+i*image.width+j));
		printf("\n");
	}*/
	PRINT_FUNCTION_END_INFO();
}

/**
 * Dectect object use Multi-scale Haar cascade classifier
 */
void HaarCasadeObjectDetection()
{
	double Scale, StartScale, ScaleWidth, ScaleHeight;
	unsigned int i, itt, x, y, width, height, step;
	PRINT_FUNCTION_INFO();
	// get start scale
	ScaleWidth = image.width/haarcascade.size1;
	ScaleHeight = image.height/haarcascade.size2;
	if(ScaleHeight < ScaleWidth)
		StartScale = ScaleHeight; 
	else
		StartScale = ScaleWidth;
	printf("StartScale = %lf\n", StartScale);
	itt = (unsigned int)ceil(log(1.0/StartScale)/log(haarcascade.ScaleUpdate));
	printf("Total itt = %lf / %lf = %d\n", \
			log(1.0/StartScale), log(haarcascade.ScaleUpdate), itt);
	printf("Start iteration.....\n");
	for(i=0; i<itt; i++)
	{
		printf("itt %d ", i+1);
		Scale = StartScale * pow(haarcascade.ScaleUpdate, i);
		step = (Scale>2.0) ? Scale : 2.0;
		printf("scale %lf step %d ", Scale, step);
		width = (unsigned int)(haarcascade.size1*Scale);
		height = (unsigned int)(haarcascade.size2*Scale);
		printf("width %d height %d\n", width, height);
		// Make vectors with all search image coordinates 
		Point point;
		vector<Point> points;
		//0:step:(IntegralImages.width-width-1),
		//0:step:(IntegralImages.height-height-1)
		for(x=0; x<image.width-width; x+=step)
		{
			for(y=0; y<image.height-height; y+=step)
			{
				point.x = x, point.y = y;
				points.push_back(point);
			}
		}
		OneScaleObjectDetection(points, Scale, width, height);
		printf("Already found %d object(s)\n", objects.size());
	}
	PRINT_FUNCTION_END_INFO();
}

double GetSumRect(int type,
                  unsigned int x, unsigned int y,
                  unsigned int w, unsigned int h)
{
	if(w==0 || h==0)
		return 0;
	w--; h--;
	//DPRINTF("(%d %d) %d %d\n", x, y, w, h);
	assert(x+w<image.width && y+h<image.height);
	if(type == II1)
		return	*( image.idata1+(y+h)*image.width+(x+w) )
			-	*( image.idata1+(y+0)*image.width+(x+w) )
			-	*( image.idata1+(y+h)*image.width+(x+0) )
			+	*( image.idata1+(y+0)*image.width+(x+0) );
	if(type == II2)
		return	*( image.idata2+(y+h)*image.width+(x+w) )
			-	*( image.idata2+(y+0)*image.width+(x+w) )
			-	*( image.idata2+(y+h)*image.width+(x+0) )
			+	*( image.idata2+(y+0)*image.width+(x+0) );
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
	PRINT_FUNCTION_INFO();
	printf("Total %d rects to find\n", points.size());
	for(i=0; i<points.size(); i++)
	{
		for(i_stage=0; i_stage<haarcascade.stages.size(); i_stage++)
		{
			//DPRINTF("----Stage %d----", i_stage);
			Stage &stage = haarcascade.stages[i_stage];
			double StageSum =  0.0;
			for(i_tree=0; i_tree<stage.trees.size(); i_tree++)
			{
				Tree &tree = stage.trees[i_tree];
				//DPRINTF("----Tree %d----\n", i_tree);
				StageSum += TreeObjectDetection(tree, Scale, points[i], width, height);
			}
			if(StageSum > stage.threshold)
				continue;
			else
				break;
		}
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
	PRINT_FUNCTION_END_INFO();
}

/**
 * get the current haar-classifier's value (left/right)
 */
double TreeObjectDetection(Tree& tree, double Scale, Point& point,
                           unsigned int width, unsigned int height)
{
	unsigned int i_rect;
	double total_x, total_x2, moy, vnorm;
	double InverseArea = 1.0/(width*height);
	double Rectangle_sum = 0.0;
	//PRINT_FUNCTION_INFO();
	// get the variance of pixel values in the window
	total_x = GetSumRect(II1, point.x, point.y, width, height);
	total_x2 = GetSumRect(II2, point.x, point.y, width, height);
	moy = total_x*InverseArea;
	vnorm = total_x2*InverseArea - moy*moy;
	vnorm=(vnorm>1.0)?sqrt(vnorm):1.0;
	// for each rectangle in the feature
	for(i_rect=0; i_rect<3; i_rect++)
	{
		Rect &rect = tree.rects[i_rect];
		//DPRINTF("[%d %d] %d %d\n", rect.x, rect.y, rect.width, rect.height);
		unsigned int RectX = rect.x * Scale + point.x;
		unsigned int RectY = rect.y * Scale + point.y;
		unsigned int RectWidth = rect.width * Scale;
		unsigned int RectHeight = rect.height * Scale;
		Rectangle_sum += GetSumRect(II1, RectX, RectY, RectWidth, RectHeight) * rect.weight;
	}
	Rectangle_sum *= InverseArea;
	if(Rectangle_sum < tree.threshold*vnorm)
		return tree.left_val;
	else
		return tree.right_val;
}

void ShowDetectionResult()
{
	FILE *fout;
	unsigned int i_obj;
	PRINT_FUNCTION_INFO();
	printf("Total %d object(s) found\n", objects.size());
	fout = fopen("result.txt", "w");
	for(i_obj=0; i_obj<objects.size(); i_obj++)
	{
		Rect &rect = objects[i_obj];
		fprintf(fout, "%d %d %d %d\n", rect.x, rect.y, rect.width, rect.height);
	}
	fclose(fout);
	PRINT_FUNCTION_END_INFO();
}

int main()
{
	mshcd("./tools/gray_img.raw", "haar.txt");
	return 0;
}

