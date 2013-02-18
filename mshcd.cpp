
void mshcd(imagefile, haarcasadefile)
{
	GetHaarCasade(haarcasadefile);
	GetIntergralImages(imagefile);
	HaarCasadeObjectDetection(image, haarcasade);
}

GetHaarCasade(filename)
{
	stages = ;
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
	}
}

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

