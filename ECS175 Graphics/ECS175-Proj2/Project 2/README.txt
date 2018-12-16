Tyler Welsh, 912341695, tylwelsh@ucdavis.edu

Compile: cd to directory and make

Run: No command line arguments. Just ./out and follow the console prompt.

Project:
- General Functions: Fully Functioning - Hardcoded size. NDC works as far as I can tell, everything stays in perspective. I/O works in world coords.
- Scale: Functioning - Could be bugged though. Taking Polygon #0 in default coords and scaling by 2 creates an odd shape. Did not test coords by hand
- Translate: Fully Functioning - Translates objects perfectly fine. However entering a translation vector in the 1000's was making it crash. So don't do that.
		- Is breaking when translating by some coordinates. Causes y to become -1. Have not fully tested which translate break it. Seems to be large numbers
		   Using like 50, 100, 150, those numbers work as well as using the centroid
- Rotate: Fully Functioning - Input is first axis point, second axis point, then angle. Use the right click menu to undraw the axis.
- Orthogonal Projection: Fully Functioning - 3 Subwindows. Top left is XY, top right is XZ, bottom left is YZ.
- UI: Up and running, see below for more details.

General Functions:
- Console U/I is fully functional and interactive. No error handling is setup, so please only enter valid inputs.
	- There is a bug in freeGLUT that I could not overcome. cin occasionally causes freeglut to crash. For some reason, translate scale and rotate worked.
		But when trying to say, right click > save file > get file name using cin, freeglut would then crash.
- File I/O is fully functioning. See below for details.

Menu / UI:
- To use menu, right click the bottom right 'sub' window.
- All options display output and/or give control to the console. If console gains control with cin, the glut menu will just kinda hang there.
- Select a Polygon to interact with via the dropdown. Polygon # corelates to the order in which it is entered in console or file.

Extra Credit:
- Animation: Rotates the CURRENT POLYGON around the axis (0,0,0)-(500,500,500) by 45 degrees every 1 second.
    - Could not get animation to stop. Therefor you must quit.

Notes:
- Up to 3 polygons.
- Polygons are constructed in order of entered coordinates.

File Name:
POLYS.dat

File Format:
	Number of Polygons
	Number of vertices in polygon
		X Y
		X Y
		X Y
	.......
	How edges are connected
		1 2
		1 3
		3 4
	.......
	Number of vertices in polygon
		X Y
		X Y
	.......