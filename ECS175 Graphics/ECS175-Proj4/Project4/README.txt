Tyler Welsh, 912341695, tylwelsh@ucdavis.edu

Compile: cd to directory and make

Run: No command line arguments. Just ./curves and follow the console prompt.

Project:
	Bezier Curve - Finished Completely. Good output. Only 1 or 2 stray pixels.
	B-Spline Curve - Finished Completely. Good output.
	UI - Finished. Can add, delete, and change parameters. See below for details.
	Manual - You're reading it

Bezier Curve: Implementation starts Line 527 in drawCurves() function
	Implements the algorithm given in class (commented on line 529). Gives output as intended.

B-Spline Curve: Implementation starts Line 550 in drawCurves() function
	Implements the algorithm given in class and office hours (commented on line 567). Gives output almost as intended.
 
Menu / UI: Implementation for menu is in createMenu() Line 215 and menu() Line 244
	Right click anywhere to use the menu. Will cause the GLUT window to hang if console requires input.
	Select a curve, maximum of 6 from the drop down menu.
	Loads and Saves the scene just fine using the drop down menu.
	Use Display... to display all the info for the currently selected curve.
	Manipulation:
		Ability to Add, Delete, and Change a control point. Hands the control over to the console. Follow console prompt
		Ability to change the K Value, gives control to console. Follow console prompt.
		Ability to Add, Delete and Change the knots of a B-Spline. Hands control to the console. Follow console prompt.
		Note: There is some checking for knots. If there are more than n+k knots, you will be alerted. If k is altered and there
			exists more than n+k knots, the last knot in the list will be deleted.

Notes:
	There may or may not be some crashing. If there is ever an exception thrown due to freeglut.dll, it is out of my control.
	freeglut will still hang crash occasionally when dealing with cin and menus. 


File Name:
CURVE.dat

File Format:
	Resolution
	#Of Curves
	Type_Of_Curve (Bz for Bezier, Bs for B Spline)
	#Of Points
	Points
	....
	Type_Of_Curve
	#Of Points
	Points
	....
	_IF_CURVE_IS_BS_
	Bs
	#Of Points
	K Value
	Knots Defined? (0 | 1)
	Points
	....
	Knot Values
	
	REPEAT FOR # OF CURVES