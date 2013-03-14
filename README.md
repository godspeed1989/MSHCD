MSHCD
=====
The Multi-Scale Haar Cascade Detection    

The Multi-Scale Haar Cascade Detection (MSHCD) is a object detection algorithm.    
It was introduced by Viola-Jones in 2001.    

Source
======
C implementation in `c` folder    
CPP implementation in `cpp` folder    

Usage for C/CPP version
=======================
Step 1. Generate raw grayscale image file.    
	cd ./tools    
	make    
	./raw imagefilename    
Step 2. Run detection.    
	cd ./c or ./cpp    
	make    
	./mshcd    

Haar cascade features data
==========================
The trained data is gotten from OpenCV project.    
And converted into C format that easy to be read in program.    
haar_alt.txt    
haar_default.txt    
File format:    
	sample-size1 sample-size2    
	num-of-stages    
	num-of-trees-at-stage-(i+1)    
	i-stage i-tree 1  1  x  y  width  height  weight    
	i-stage i-tree 1  2  x  y  width  height  weight    
	i-stage i-tree 1  3  x  y  width  height  weight //may exist    
	i-stage i-tree 1  0  //terminal, not used    
	i-stage i-tree 1  tree-i-threshold    
	i-stage i-tree 1  left_val    
	i-stage i-tree 1  right_val    
	......    
	i-stage stage_threshold    
	num-of-trees-at-stage-(i+1)    
	......    

Reference
=========
The java implementation in `java` folder    
http://code.google.com/p/jviolajones    
The matlab implementation in `matlab` folder    
http://www.mathworks.com/matlabcentral/fileexchange/29437-viola-jones-object-detection    
The javascript implementation in `javascript` folder    
https://github.com/foo123/HAAR.js    

