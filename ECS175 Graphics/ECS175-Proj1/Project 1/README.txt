Tyler Welsh, 912341695, tylwelsh@ucdavis.edu

Compile: cd to directory and make

Run: No command line arguments. Just ./out and follow the console prompt.

Project:
DDA - Fully functioning as tested by me					 || Line: 151 main.cpp	
Bresenham - Fully functioning as tested by me		 || Line: 198 main.cpp	
Rasterizing - Mostly working. There are some edge cases that may produce no line with side by side polygons, but its 95% working		|| Line: 578 main.cpp	
	I ran out of time and/or got to lazy to fully fix the vertex issue. 
Scaling - Fully functioning, but possible error in some pixel positioning		|| Line: 758 main.cpp	
	Scaling, do not use fractions, decimals only.
Translating - Fully functioning			|| Line: 715 main.cpp	
Rotating - Fully functioning	|| Line: 728 main.cpp	
Clipping - Sorta working? It apparantly is masking, not clipping. It will clip the window down, deleting content from the PIXEL_BUFFER and not truly editing any polygons.		|| Line: 794 main.cpp
	Use menu to reset to original view window. Like stated above, clipping does not truly edit any polygons, just the display window.
		This means if you clip, then save, then load, the original polygons will load without the clipping. Clipping is not saved to file.
			ALSO IMPORTANT NOTE: Strict Ordering. Enter your clip window Bottom Left, Bottom Right, Top Left, Top Right ONLY FOUR COORDS
		Clipping does not delete the axis
	VERY IMPORTANT CLIPPING NOTE: So using cin in menu() to get clip window input was throwing exceptions. So, how I'm overcoming this is just using the file CLIP_COORDS.dat
	to input a clipping window. Enter your coords with the strict ordering above into the file and use clip in the menu

General Functions:
- Console U/I is fully functional and interactive. No error handling is setup, so please only enter valid inputs.
	- Lets you define line algorithm, load from file, or enter polygons/lines coordinate by coordinate.
- File I/O is fully functioning. See below for details.

Menu:
- To use menu, right click the GLUT window
- Manipulation menu options give control back to console. GLUT will hang and float the menu.
- Select a Polygon to interact with via the dropdown. Polygon # corelates to the order in which it is entered in console or file.

Notes:
- Up to 6 polygons and lines COMBINED can be made. Lines and Polygons use same data structure.
- Do not try to select a polygon that has not been made. Error handling wasn't implemented.
- Polygons are constructed in order of entered coordinates.
- Edges per polygons are displayed for debugging/viewing purposes upon display.
- Currently no way to redefine height and width without reloading.
- I did not include a menu function to add polygons on the fly, I'm sorry. Must reload the program to get new polygons via file or console.

File Name:
POLYS.dat

File Format:
	Width Height
	Number of Polygons
	Number of vertices in polygon
		X Y
		X Y
		X Y
	.......
	Number of vertices in polygon
		X Y
		X Y
	.......