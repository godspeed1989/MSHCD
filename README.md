MSHCD
=====
The Multi-Scale Haar Cascade Detection (MSHCD) is an object detection algorithm.    
Introduced by Viola-Jones in 2001.    

Usage for C/CPP version
=======================
Step 1. Generate raw gray scale image file.    
```
	cd ./tools    
	make    
	./raw imagefilename    
```
Step 2. Run detection.    
```
	cd ./c or ./cpp    
	make    
	./mshcd imagefilename cascascadefile    
```

Haar cascade features data
==========================
The trained data is gotten from OpenCV project.    
And converted into C format that easy to be read in program.    
haar_alt.txt    
haar_default.txt    
File format:    
```
	sample-size1 sample-size2    
	num-of-stages    
	num-of-trees-at-stage-(i)    
/	i j_tree 1  1  x  y  width  height  weight    
.	i j_tree 1  2  x  y  width  height  weight    
.	i j_tree 1  3  x  y  width  height  weight //may exist    
.	i j_tree 1  0  //terminal, not use    
.	i j_tree 1  j_tree-threshold    
.	i j_tree 1  left_val    
\	i j_tree 1  right_val    
	......    
	$i stage_threshold    
	num-of-trees-at-stage-(i+1)    
	......    
```

Reference
=========
The java implementation in `java` folder    
http://code.google.com/p/jviolajones    
The matlab implementation in `matlab` folder    
http://www.mathworks.com/matlabcentral/fileexchange/29437-viola-jones-object-detection    
The javascript implementation in `javascript` folder    
https://github.com/foo123/HAAR.js    

