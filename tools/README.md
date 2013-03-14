##Tools

###draw.cpp
Draw the result of found objects.    
	Usage: ./draw filename image    
'filename' contents:    
x1 y1 w1 h1    
x2 y2 w2 h2    
.....    

###raw.cpp
Convert image to `raw` format.    
The raw image is a grayscale image, 1 byte per pixel(0..255).    
Raw image file format:    

|  width/cols(4 bytes)  |  height/rows(4 bytes)  |  data(cols*rows bytes)  |    

###show.cpp
Show Haar features at each cascade stage.    
This program is used to illustrate the haar cascade features.    

