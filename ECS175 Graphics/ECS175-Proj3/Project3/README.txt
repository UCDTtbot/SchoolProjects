Tyler Welsh, 912341695, tylwelsh@ucdavis.edu

Compile: cd to directory and make

Run: No command line arguments. Just ./out and follow the console prompt.

Project:
	Phong - Complete as far as I can tell testing against hand written tests
	Gouraud - Mostly Complete. Slightly Broken but it colors. See Bwloe
	Painter's Algorithm - Sorta complete? Implemented basic face sorting and draw them to screen based on depth order. See below.
	Halftone - Complete, see below. The "off" switch is broken.
	
Phong: See P3_Funcs.cpp, phong()
	Implemented as according to the equation given in the PDF and lecture. Input is defaulted and can be found at line 78 in main.cpp. Use the console UI to input new parameters.
	Output has been tested using an excel file. Output of my program matches that of the excel output. However, colors show up funny but that could be due to my raster being broken.
	
Gouraud: See raster() at line 1255 and the associated raster functions below it: rasterLines(), rastPix(), XY|XZ|YZ|raster, fillDDA, scanIntens
	Sort of works. My old raster made this a problem. I draw the colors of the edges via setEdgeIntense line 611. The fill in is then carried out by raster(). Because I didn't use slope equations to
	calculate x-intercepts, I use a silly little way around it. Fill in is atomic and only uses the screen. I fill a temp buffer with -1's where edges exist. As I scan line and it -1's, I fill an x-intercept vector.
	At the end, I draw a line between the first and last x-intercept, using the Gouraud equation to calculate the intensities between edge points.
	Little sketchy, but I did 75% of this program in the span of 24 hours soooo yaaaa.
	---------------NOTE--------------- TO TURN OFF FILLING, COMMENT OUT raster() ON LINE 232

Painters: See drawFaces() line 580 and other associated functions: drawLines()
	I implemented this in the most simple way possible. I take the faces of the polygon, one by one, and sort them into a vector by their z depth. This is the only check I do, so there COULD be overlapping
	problems. After the faces have been sorted by depth, I simply draw them, edge by edge, face by face. Rasterization is carried out afterwords.

Halftone:
	Starting line 1662. Only flips "on"
	Currently takes the intensity of a 3x3 section of pixels, averages out the intensities of that 3x3, and sets that 3x3 chunk with the new avg intensity to create one "pixel" of color.

Menu / UI:
	Same as my other programs. Right click the bottom right black window to open drop down window. Will cause the openGlut windows to hang when it gives control to the console for inputs.
	Saving has been disabled, but loading can still be done on the fly.
	--------------NOTE--------------- THERE IS CURRENTLY A BUG IN FREEGLUT. On Windows 8, VS2015, freeglut.dll throws an exception after using Phong Input. I have not tested on CSIF yet
														but this is something I CANNOT fix, as it is a problem with freeglut.

Extra Credit:
	Did not attempt non-plainer, CVM, or animation

Notes:
	See marked notes above. There is a freeglut crash I cannot fix in the UI.

File Name:
POLYS.dat

File Format:
	Number of Polygons
	Number of vertices in polygon
		X Y Z - VERTICES
		X Y Z
		X Y Z
	.......
		X Y Z - VECTOR NORMALS
		X Y Z
		X Y Z
	.......
	Number of Edges 
		1 2	 - POINTS CONNECTING EDGES
		1 3
		3 4
	.......
	Number of Surfaces
		1 2 3 - POINTS MAKING A FACE
		2 3 4
		1 3 4
	......
		X Y Z - SURFACE NORMALS
		X Y Z
		X Y Z
	......
	REPEAT FROM # OF VERTS