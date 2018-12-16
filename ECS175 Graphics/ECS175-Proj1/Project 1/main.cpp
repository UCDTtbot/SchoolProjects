#include <GL/glut.h>
#include <stdio.h>
#include <iostream>
#include <utility>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <string.h>
#include <sstream>
#include <vector>
#include "polygon.h"

using namespace std;

float *PIXEL_BUFFER; // [width * height * RGB] | [x * 3 + y * width * 3] for 0, then add 1 and then 2 
float *tempPoly; //Used for raster
float RGB = (1 / 255);  //Used for RGB conversion
static int MainWindow;  //Global for holding the mainWindow ID
static int CURRENT_POLY = 1; //Keeps track of currently selected Poly
int drawAlg = 1; //1 DDA, 2 Bres | defaulted to DDA
int WIDTH, HEIGHT, MID_X, MID_Y; //EXTERNED VARIABLES FROM WINDOW.H
//For Clipping
static int X_0, X_1, X_2, X_3;  //Default testing values because I got sick of entering values
static int Y_0, Y_1, Y_2, Y_3;
//Menu Vairables
static int menuID, subMenuID1, subMenuID2;  //Globals for holding menu IDs
static int menuValue;   //Menu item selected
//File I/O variables
char *fileName = "POLYS.dat";  //Default filename. Possibly will not change
char *clipData = "CLIP_COORDS.dat";
ofstream outputFile; //output file object
ifstream inputFile; //input file object
void loadFile();    //Function for loading from file
void saveFile();    //Function for saving to file


const float  PI_F = 3.14159265358979f; //PI for degrees to radians
const float D_TO_R = PI_F / 180;    //Use to convert degrees_to_radian
//Polygon setup
static int totalPolys = 0;  //Total polys made
static int maxPolys = 6;    //Max number of polys allowed to be made in this program
vector<polygon> Poly;   //Vector for holding polys that have been made
//Functions for setting up OpenGL
void display(); //OpenGL Display function
void init();    //function to initiate many things in my program
//For Menu
void createMenu();  //function to create the menus
void menu(int num); //OpenGL menu callback function



//Line and Polygon Functions
void setPixel(float x, float y, float R, float G, float B); //Function to set pixels in pixel buffer. Takes it coordinate and RGB color value
void lineDDA(float X0, float Y0, float X, float Y, float R, float G, float B);  //Draws line from initial point to final point. Also takes RGB color
void lineBres(int X0, int Y0, int X_End, int Y_End, float R, float G, float B); //See above
void scanLine();       //Scan Line function for raster
void consoleMenu(); //Function for setting up the console menu
void drawLines();   //Function that draws lines to pixel buffer using either lineDDA or lineBres
void raster();      //Rasterization function
void translate(int x, int y);   //Translation func
void rotate(float angle);       //Rotation func
void scale(float alpha, float beta);    //Scaling func
void clip();    //Clipping Func
void clipLoad(); //Load stuff for clipping
void rasterLines(float X0, float Y0, float X_End, float Y_End); //function for drawing lines in tempPoly array
void rastPix(float x, float y); // func for rasterize pixels
void drawAxis();

int main(int argc, char *argv[])
{
    char fileLoad; //Variable to load from file or not
    cout << "Welcome.\n";
    //Choose how to draw lines
    cout << "Please choose a line drawing algorithm: \n";
    cout << "1. DDA\n2. Bres\n"; cin >> drawAlg;
    while (drawAlg < 1 || drawAlg > 2)
    {
        cout << "Invalid input...\n";
        cout << "Please choose a line drawing algorithm: \n";
        cout << "1. DDA\n2. Bres\n"; cin >> drawAlg;
    }
    //Ask to load
     cout << "Would you like to load from file? (Y/N): "; cin >> fileLoad;

    //Prep file I/O
    if (fileLoad == 'Y' || fileLoad == 'y')
    {
        loadFile();
    }
    else //if user decides to not load from file, get console menu running
    {
        consoleMenu();
    }
    //Window setup below
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowPosition(250, 100);
    glutInitWindowSize(WIDTH, HEIGHT);
    MainWindow = glutCreateWindow("Project 1");

    createMenu();//Initialize Menu and Window stuffs
    init();

    //Starts the main GLUT loop
    glutDisplayFunc(display);
    //Lists edges to console to see what poly's were made
    for (int step = 0; step < totalPolys; step++)
    {
        Poly[step].listEdges();
    }

    glutMainLoop();
    
    return 0;
}
//Initialize window color and PixelBuffer
void init()
{
    MID_X = WIDTH / 2; MID_Y = HEIGHT / 2;  //Gets the mids for use in the coordinate system
    glClearColor(1.0, 1.0, 1.0, 0.0);
    PIXEL_BUFFER = new float[WIDTH * HEIGHT * 3]; //Initiate pixel buffer
    for (int x = 0; x < WIDTH * HEIGHT * 3; x++)
        PIXEL_BUFFER[x] = 1.0f;  //Set initial buffer to a blank white backround
    drawAxis();
    raster();
    drawLines();    //Draw the lines to the pixelBuffer
}

void drawAxis()
{
    lineDDA(-MID_X, 0, MID_X - 1, 0, 0.0f, 0.0f, 0.0f); //Setup horizontal axis
    lineDDA(0, -MID_Y, 0, MID_Y - 1, 0.0f, 0.0f, 0.0f); //Setup vertical axis
}

void setPixel(float x, float y, float R, float G, float B)
{   //Backround is Init to 1.0f = white, set pixel RGB to 0.0f for black
    for (int z = 0; z < 3; z++)
    {
        if(z == 0)
            PIXEL_BUFFER[(((int)round(x) + MID_X) * 3 + ((int)round(y) + MID_Y) * WIDTH * 3) + z] = R;  //Set R
        else if (z == 1)
            PIXEL_BUFFER[(((int)round(x) + MID_X) * 3 + ((int)round(y) + MID_Y) * WIDTH * 3) + z] = G;  //Set G
        else if (z == 2)
            PIXEL_BUFFER[(((int)round(x) + MID_X) * 3 + ((int)round(y) + MID_Y) * WIDTH * 3) + z] = B;  //Set B
    }
       
}


//Parameters of lineDDA are the beginning and end points of our wanted line.
void lineDDA(float X0, float Y0, float X_End, float Y_End, float R, float G, float B)
{
    /*
    *       Note: delta_X and delta_Y found from (X0, Y0), (X, Y)
    *           i is horizontal axis, j is vertical axis
    *   Line Drawing Algorithm - DDA Algorithm (Digital Differential Analyzer) 
    *       for 0 < m < 1
    *   Move left to right (x1 < x2) and determine what pixel is closest to L (for each column) 
    *   X0 to Xn, Yn = 1 + Xn*m, where m is slope found in delta_X / delta_Y
    *   int (trunc) the y, and MakePix(i, j) using the Xn and the Yn
    *       for 1 < m < inf
    *   Same algorithm as above, but swap X's with Y's and i for j
    *   Y0 to Yn, Xn = 1 + Yn*m, where m is the slope found in delta_X / delta_Y
    */
    float delta_X = X_End - X0, delta_Y = Y_End - Y0, steps; //delta's for X and Y and the number of steps we take in the for loop depending on the 0 < m < 1 stuff
    float xInc, yInc, Xn = X0, Yn = Y0; //Inc is the increment in the equations for Xn+1 and Yn+1 (either 1 or +/- m)

    if (delta_Y == 0) //Slope is undefined ---- vertical line
    {
        steps = fabs(delta_X);  //Steps toi move up the line
        xInc = float(delta_X) / float(steps);   //Increasing value for X
        setPixel(round(Xn), round(Yn), R, G, B); //Set init point
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc;
            setPixel(round(Xn), round(Yn), R, G, B);
        }//(100, 150) (0, 150)
    }
    else //Slope is defined
    {
        if (fabs(delta_X) > fabs(delta_Y)) //number of steps to take in the for loop (the delta) 
            steps = fabs(delta_X);   //This if determines 0 < m < 1 or 1 < m < inf
        else
            steps = fabs(delta_Y);
        xInc = float(delta_X) / float(steps); //will set to either 1 or m, for the equation for Xn+1 = x + 1/m or Yn+1 = y+1/m and so forth
        yInc = float(delta_Y) / float(steps);

        setPixel(round(Xn), round(Yn), R, G, B);//Creates initial pixel
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc; //Increment X and Y by their respective values
            Yn += yInc;
            setPixel(round(Xn), round(Yn), R, G, B);
        }
    }
}

void lineBres(int X0, int Y0, int X_End, int Y_End, float R, float G, float B)
{
    /*
    *   Line Drawing Algorithm - Bresenham's Algorithm
    *       You simply decide if Yn should be incremented while Xn is incremented
    *   Solved with a recursive function where,
    *   IF (Pi < 0) THEN Yi+1 = Yi, ELSE Yi+1 = Yi + 1
    *   Where Pi+1 = Pi + 2*delta_Y - 2*delta_X*(Yi+1 - Yi)
    *   And P1 = 2*delta_Y - delta_X
    *       for 1 < m < inf
    *   Swap the X's and Y's and i for j
    */
    float m; //Slope for determining which case to use
    int delta_X = fabs(X_End - X0), delta_Y = fabs(Y_End - Y0); //delta values
    int P = 0;  //principal
    int Xn, Yn, xInc = 1, yInc = 1; 

    if (delta_X > 0) //Can't divide by 0
        m = delta_Y / float(delta_X);
 
    if(delta_X == 0)    //Case for vertical line
    {
        if (Y0 > Y_End) //Case where its a "negative" slope, swap Yn and Y_End basically
        {
            Yn = Y_End; 
            Xn = X0;    
            Y_End = Y0; 
        }
        else
        {
            Xn = X0;    //Set Xn and Yn to init, these will be used in setPixel
            Yn = Y0;
        }

        setPixel(Xn, Yn, R, G, B);
        P = 2 * delta_Y - delta_X; //Don't really need this but hey, gotta stay in the bres spirit
        while (Yn < Y_End)
        {
            Yn += yInc;
            if (P > 0) //If P > 0, then use the 2 * delta_X
                P += 2 * delta_X;   //NOTE: This case is kinda funky, as we are just drawing a vertical line.
            else
            {
                Yn += yInc; //again... unneeded. its a straight line
                P += 2 * (delta_X - delta_Y);
            }
            setPixel(Xn, Yn, R, G, B);
        }
    }
    else if (m > 1) // m > 1 case | SWAP Xs AND Ys ROLES
    {
        if (Y0 > Y_End) //If the initial Y is greater than the final Y, swap it the other way to go left to right
        {
            Yn = Y_End;
            Xn = X_End;
            Y_End = Y0;
            if (X0 < X_End) //If the slope is negative basically, we'll be decreasing instead
                xInc = -1;
        }
        else if (X0 > X_End)    //If init X is greater, same as above
        {
            Yn = Y0;
            Xn = X0;
            xInc = -1;  //Slope is nega here no matter what so ya
        }
        else
        {
            Xn = X0;
            Yn = Y0;
        }
        setPixel(Xn, Yn, R, G, B);
        P = 2 * delta_X - delta_Y;

        while (Yn < Y_End) //Loop from beginning X to End
        {
            Yn++;
            if (P < 0)//if(P < 0), do Pi+1 = Pi + 2 * delta_Y
                P += 2 * delta_X;
            else    //ELSE Pi+1 = Pi + 2*(delta_Y - delta_X)
            {
                Xn += xInc;
                P += 2 * (delta_X - delta_Y);
            }//Above if statement is for determining the "P" or principal (i think). This decides whether or not we go up 1 in the Y axis
            if (Yn == MID_Y)
                Yn--;
            setPixel(Xn, Yn, R, G, B);
        }
    }
    else //BASE CASE 0 < M < 1
    {       // 0 < m < 1
        if (X0 > X_End)             
        {               //Again, like above, swap Xknot and X end if needed 
            Xn = X_End;
            Yn = Y_End;
            X_End = X0;
            if (Y0 < Y_End) //Check if negative slope
                yInc = -1;
        }
        else if (Y0 > Y_End) //This is case for a negative slope in respect to the Y axis, so yIncrement is set to -1
        {
            Xn = X0;           
            Yn = Y0;            
            yInc = -1;          
        }
        else
        {
            Xn = X0;
            Yn = Y0; //Otherwise Xn and Yn are just initial
        }
        setPixel(Xn, Yn, R, G, B);
        P = 2 * delta_Y - delta_X;

        while (Xn < X_End) //Loop from beginning X to End
        {
            Xn++;
            if (P < 0)//if(P < 0), do Pi+1 = Pi + 2 * delta_Y
                P += 2 * delta_Y;
            else    //ELSE Pi+1 = Pi + 2*(delta_Y - delta_X)
            {
                Yn += yInc;
                P += 2 * (delta_Y - delta_X);
            }//Above if statement is for determining the "P" or principal (i think). This decides whether or not we go up 1 in the Y axis
            if (Yn == MID_Y)
                Yn--;
            setPixel(Xn, Yn, R, G, B);
        }
    }
}

void display()
{ 
    //Function that is looped through
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();   

    //Alright edge table time
    //EdgTable.init(HEIGHT);

    //Draws the PixelBuffer to screen
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, PIXEL_BUFFER);

    glFlush();
}


//Menus will be used for  manipulating (scaling, translation, and rotation)
//Menu will be able to select a polygon
void createMenu()
{
    //First create submenu
    subMenuID1 = glutCreateMenu(menu);  //SubMenu for Polygon IDs
    glutAddMenuEntry("Polygon #1", 1);
    glutAddMenuEntry("Polygon #2", 2);
    glutAddMenuEntry("Polygon #3", 3);
    glutAddMenuEntry("Polygon #4", 4);
    glutAddMenuEntry("Polygon #5", 5);
    glutAddMenuEntry("Polygon #6", 6);
    subMenuID2 = glutCreateMenu(menu);  //SubMenu for Manipulate
    glutAddMenuEntry("Scale", 7);
    glutAddMenuEntry("Translate", 8);
    glutAddMenuEntry("Rotate", 9);
    menuID = glutCreateMenu(menu);  //Setup main Menu
    glutAddSubMenu("Polygon IDs", subMenuID1);  //Attaches to subMenu 1
    glutAddSubMenu("Manipulate", subMenuID2);   //Attaches to subMenu 2
    glutAddMenuEntry("Add Polygon", 14);
    glutAddMenuEntry("Save Polygons", 10);
    glutAddMenuEntry("Load Polygons", 11);
    glutAddMenuEntry("Clip Viewport", 12);
    glutAddMenuEntry("Reset View Window", 13);
    glutAddMenuEntry("Quit", 0);
    glutAttachMenu(GLUT_RIGHT_BUTTON); //Binds the menu to Right Mouse Click
}

void menu(int num)
{   
    //MENU CALLBACK HANDLES
    if (num == 0)   //If Quit is selected, exit
    {
        glutDestroyWindow(MainWindow);
        exit(0);
    }
    else if (num == 1)  //Polygon #1
        CURRENT_POLY = 1;
    else if (num == 2)  //Polygon #2
        CURRENT_POLY = 2;
    else if (num == 3)  //Polygon #3
        CURRENT_POLY = 3;
    else if (num == 4)  //Polygon #4
        CURRENT_POLY = 4;
    else if (num == 5)  //Polygon #5
        CURRENT_POLY = 5;
    else if (num == 6)  //Polygon #6
        CURRENT_POLY = 6;
    else if (num == 7)  //Scale
    {
        float x, y;
        cout << "Please enter the scale vector: \nAlpha: "; cin >> x;
        cout << "Beta: "; cin >> y;
        cout << "Scaling and redrawing....\nReturn to GLUT window\n";
        scale(x, y);
        init();
    }
    else if (num == 8)  //Translation
    {
        int x, y;
        cout << "Please enter the translation vector: \nX: "; cin >> x;
        cout << "Y: "; cin >> y;
        cout << "Translating and redrawing....\nReturn to GLUT Window\n";
        translate(x, y);
        init();
    }
    else if (num == 9)  //Rotation
    {
        int angle;
        cout << "Please enter an angle in degrees: \nAlpha: "; cin >> angle;
        cout << "Rotating and redrawing...\nReturn to GLUT Window\n";
        rotate(angle);
        init();
    }
    else if (num == 10)  //Save
        saveFile();
    else if (num == 11)  //Load
    {
        for (int step = 0; step < totalPolys; step++)
        {
            Poly[step].clearEdges();
        }
        Poly.clear();
        loadFile();
        init();
    }
    else if (num == 12)  //Clipping
    {
        clipLoad();
        clip();
        cout << "Done Clipping.\n";
        drawAxis();
    }
    else if (num == 13)  //Reset viewport to original
    {
        init();
    }
    else if (num == 14)
    {
        //float x = 0, y = 0;
        //int totalVerts = 0, curID = 0;
        //curID = totalPolys;
        //if (totalPolys < 6)
        //{
        //    polygon newPoly(curID);
        //    cout << "Please enter number of vertices in your polygon: "; cin >> totalVerts;
        //    for (int step = 0; step < totalVerts; step++)
        //    {
        //        cout << "X coord: "; cin >> x;
        //        cout << "Y coord: "; cin >> y;
        //        newPoly.addVert(x, y);
        //    }
        //    Poly.push_back(newPoly);
        //    totalPolys++;
        //    Poly[curID].populateEdges();
        //    Poly[curID].getCentroid();
        //}
        //else
        //    cout << "Too many Poly's. Cannot add more\n";
        cout << "cin >> is causing an exception to be thrown in glutMainLoop(). Aborting Add.\n";
        init();
    }
    else //Fallback function
    {
        menuValue = num;
    }

  //  Poly[CURRENT_POLY - 1].getCentroid();   //Reget centroid
  //  Poly[CURRENT_POLY - 1].clearEdges();    //Clear edges
  //  Poly[CURRENT_POLY - 1].populateEdges(); //Remake edges
    for (int step = 0; step < totalPolys; step++)
    {
        Poly[step].listEdges(); //Relist edges
    }
    glutPostRedisplay(); //Repost the display function to redraw everything
}

void consoleMenu()
{
    float x = 0, y = 0;
    int decision = -1, totalVerts = 0, curID = 0;
    bool isDone = false;
    //Console Window setup
    cout << "Please enter window height: "; cin >> HEIGHT;
    cout << "Please enter window width: "; cin >> WIDTH;
    cout << "Polylines will be drawn in order of given vertices.\n";

    while (totalPolys < maxPolys && isDone == false)
    {
        cout << "You can create " << maxPolys - totalPolys << " more lines or polygons.\n";
        cout << "Would you like to create:\n";
        cout << "1. Polygon\n2. Line\n3. Finished. Draw Polygons/Lines\n"; cin >> decision;

        while (decision < 0 || decision > 3)
        {
            cout << "Invalid input\n";
            cout << "You can create " << maxPolys - totalPolys << " more lines or polygons.\n";
            cout << "Would you like to create:\n";
            cout << "1. Polygon\n2. Line\n3. Finished. Draw Polygons/Lines\n"; cin >> decision;
        }
        if (decision == 1)
        {
            //Create Polygon
            curID = totalPolys;
            polygon newPoly(curID);
            cout << "Please enter number of vertices in your polygon: "; cin >> totalVerts;
            for (int step = 0; step < totalVerts; step++)
            {
                cout << "X coord: "; cin >> x;
                cout << "Y coord: "; cin >> y;
                newPoly.addVert(x, y);
            }
            Poly.push_back(newPoly);
            totalPolys++;
            Poly[curID].populateEdges();
            Poly[curID].getCentroid();
        }
        else if (decision == 2)
        {
            //Create Line
            curID = totalPolys;
            polygon newLine(curID);
            cout << "Please enter the first vertice:\n";
            cout << "X coord: "; cin >> x;
            cout << "Y coord: "; cin >> y;
            newLine.addVert(x, y);
            cout << "Please enter the second vertice:\n";
            cout << "X coord: "; cin >> x;
            cout << "Y coord: "; cin >> y;
            newLine.addVert(x, y);
            Poly.push_back(newLine);
            totalPolys++;
            Poly[curID].populateEdges();
            Poly[curID].getCentroid();
        }
        else if (decision == 3)
        {
            cout << "Drawing Polygons";
            isDone = true;
            //Return
        }
        else
        {
            cout << "Failure. Please Exit.";
        }
    }
}

void drawLines()
{
    for (int polyStep = 0; polyStep < totalPolys; polyStep++)
    {
        polygon curPoly = Poly[polyStep];

        if (drawAlg == 2) // do Bres
        {
            for (int step = 0; step < curPoly.getVertSize() - 1; step++)
            {//Messy as shit, but it just takes (X0, Y0, X, Y, R, G, B)
                //These end up being big mean looking function calls to get X and Y cause I'm shit at programming
                lineBres(curPoly.getVert(step).getX(), curPoly.getVert(step).getY(), curPoly.getVert(step + 1).getX(), curPoly.getVert(step + 1).getY(), 0.0f, 0.0f, 0.0f);
            }
            lineBres(curPoly.getVert(curPoly.getVertSize() - 1).getX(), curPoly.getVert(curPoly.getVertSize() - 1).getY(), curPoly.getVert(0).getX(), curPoly.getVert(0).getY(), 0.0f, 0.0f, 0.0f);
        }
        else //Default to DDA
        {
            for (int step = 0; step < curPoly.getVertSize() - 1; step++)
            {
                lineDDA(curPoly.getVert(step).getX(), curPoly.getVert(step).getY(), curPoly.getVert(step + 1).getX(), curPoly.getVert(step + 1).getY(), 0.0f, 0.0f, 0.0f);
            }
            lineDDA(curPoly.getVert(curPoly.getVertSize() - 1).getX(), curPoly.getVert(curPoly.getVertSize() - 1).getY(), curPoly.getVert(0).getX(), curPoly.getVert(0).getY(), 0.0f, 0.0f, 0.0f);
        }
    }//im a shitty programmer, why is everything private i hate get functions
}


void raster()
{
    tempPoly = new float[WIDTH * HEIGHT * 3];
    int workingPoly = 0; int hitVert = 0;
    vector<int> *xInterc = new vector<int>[HEIGHT];
    /*
    Scan from y = 0 to y = MAX
    Find all X intercepts for each edge ( see book )
    From list of X intercepts, fill pairwise
        List should be someting like    Y0 = (x1, x2, x3, x4)
                                        Y1 = (x1, x2)
        Would then fill from x1 - x2, x3 - x4 for Y0 then x1 - x2 for Y1
    */

    //This is gun be a MESS with copy and pasted code due to poor forsight

    //Setup Poly Matrix

    
    for (workingPoly = 0; workingPoly < totalPolys; workingPoly++)  //Go through Poly's 1 by one
    {
        for (int x = 0; x < WIDTH * HEIGHT * 3; x++)
            tempPoly[x] = 1.0f;  //Set values for initial temp
        int X0, Y0, X_E, Y_E;
        for (int curVert = 0; curVert < Poly[workingPoly].getVertSize() - 1; curVert++) //Setup the poly in our blank matrix with -1's
        {
            X0 = Poly[workingPoly].getVert(curVert).getX();
            Y0 = Poly[workingPoly].getVert(curVert).getY();
            X_E = Poly[workingPoly].getVert(curVert + 1).getX();
            Y_E = Poly[workingPoly].getVert(curVert + 1).getY();
            rasterLines(X0, Y0, X_E, Y_E);
        }
        X0 = Poly[workingPoly].getVert(Poly[workingPoly].getVertSize() - 1).getX();
        Y0 = Poly[workingPoly].getVert(Poly[workingPoly].getVertSize() - 1).getY();
        X_E = Poly[workingPoly].getVert(0).getX();
        Y_E = Poly[workingPoly].getVert(0).getY();
        rasterLines(X0, Y0, X_E, Y_E);  //Now window is blank with poly edge's labeled as -1's

        //Now scan each line. Where we encounter a -1, add to the X intercept vector
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                if (tempPoly[x * 3 + y * WIDTH * 3] == -1)
                {
                    //Check all verts - Might fix our vertex raster problem ---- it ended up sorta fixing it sometimes, sometimes it didn't..... fuck it
                    //for (int verStep = 0; verStep < Poly[workingPoly].getVertSize(); verStep++)
                    //{
                    //    if (Poly[workingPoly].getVert(verStep).getX() == (x - MID_X) && Poly[workingPoly].getVert(verStep).getY() == (y - MID_Y))
                    //        hitVert = 1;
                    //}
                    //Else push back edge
                    if (tempPoly[(x + 1) * 3 + (y) * WIDTH * 3] != -1 && hitVert != 1)
                    {
                        xInterc[y].push_back(x);
                    }

                    hitVert = 0;
                }
            }
            //For testing uneven intercepts
            //if ((xInterc[y].size() % 2) != 0)
            //{
            //    cout << "Error in rasterizing.\nAt Y " << y << " the X intercepts were uneven. \n";
            //    cout << "Working Poly: " << workingPoly << endl; cout << "Y of " << y - MID_Y<< endl;
            //    cout << "X intercepts: ( ";
            //    for (int step = 0; step < xInterc[y].size(); step++)
            //        cout << xInterc[y][step] + MID_X << ", ";
            //    cout << ")" << endl;
            //}
            //If the x intercept vector isn't empty or uneven, go ahead and draw the lines into the real deal
            if (xInterc[y].size() != 0 && (xInterc[y].size() % 2) == 0)
            {
                for (int step = 0; step < xInterc[y].size(); step += 2) //Subtract off mid's to account for coordinate system
                {
                    lineDDA(xInterc[y][step] - MID_X, y - MID_Y, xInterc[y][step + 1] - MID_X, y - MID_Y, 0.0f, 0.0f, 0.0f);
                }
            }
        }
    }
    
}

void rasterLines(float X0, float Y0, float X_End, float Y_End)
{
    //Copy paste of lineDDA but using tempPoly instead cause I suck

    float delta_X = X_End - X0, delta_Y = Y_End - Y0, steps; //delta's for X and Y and the number of steps we take in the for loop depending on the 0 < m < 1 stuff
    float xInc, yInc, Xn = X0, Yn = Y0; //Inc is the increment in the equations for Xn+1 and Yn+1 (either 1 or +/- m)

    if (delta_Y == 0) //Slope is undefined ---- vertical line
    {
        steps = fabs(delta_X);  //Steps toi move up the line
        xInc = float(delta_X) / float(steps);   //Increasing value for X
        rastPix(round(Xn), round(Yn)); //Set init point
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc;
            rastPix(round(Xn), round(Yn));
        }//(100, 150) (0, 150)
    }
    else //Slope is defined
    {
        if (fabs(delta_X) > fabs(delta_Y)) //number of steps to take in the for loop (the delta) 
            steps = fabs(delta_X);   //This if determines 0 < m < 1 or 1 < m < inf
        else
            steps = fabs(delta_Y);
        xInc = float(delta_X) / float(steps); //will set to either 1 or m, for the equation for Xn+1 = x + 1/m or Yn+1 = y+1/m and so forth
        yInc = float(delta_Y) / float(steps);

        rastPix(round(Xn), round(Yn));//Creates initial pixel
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc; //Increment X and Y by their respective values
            Yn += yInc;
            rastPix(round(Xn), round(Yn));
        }
    }
}

void rastPix(float x, float y)
{   //Backround is Init to 1.0f = white, set pixel RGB to 0.0f for black

    //Copy paste of setPixel but for raster for the tempPoly
    for (int z = 0; z < 3; z++)
    {
        if (z == 0)
            tempPoly[(((int)round(x) + MID_X) * 3 + ((int)round(y) + MID_Y) * WIDTH * 3) + z] = -1;  //Set R
        else if (z == 1)
            tempPoly[(((int)round(x) + MID_X) * 3 + ((int)round(y) + MID_Y) * WIDTH * 3) + z] = -1;  //Set G
        else if (z == 2)
            tempPoly[(((int)round(x) + MID_X) * 3 + ((int)round(y) + MID_Y) * WIDTH * 3) + z] = -1;  //Set B
    }

}


void translate(int xIn, int yIn)
{
    //Move every point of the polygon by X and Y
    //Does not support going under (0,0)
    for (int step = 0; step < Poly[CURRENT_POLY - 1].getVertSize(); step++)
    {
        float newX = 0, newY = 0;   //New x and y
        newX = Poly[CURRENT_POLY - 1].getVert(step).getX(); newX += xIn;    //Get x, add T_X
        newY = Poly[CURRENT_POLY - 1].getVert(step).getY(); newY += yIn;    //Get y, add T_Y
        Poly[CURRENT_POLY - 1].setVert(step, newX, newY);   //set new vert
    }
}

void rotate(float angle)
{
    /*
        Rotation is very similar to scaling below. Will need to be done by centroid
        The (homogeneous) rotation matrix is:   (cos  -sin | 0)  (2)
                                                (sin   cos | 0)  (1)
                                                (-------------)  (-)
                                                (0     0   | 1)  (1)
        Steps for rotation:
            1. Translation (by -(centroid) )
            2. Rotate each vertice by the rotation matrix
            3. Translation (by (centroid) )

    */
    //First translate by -centroid
    translate(-(Poly[CURRENT_POLY - 1].getCenter().getX()), -(Poly[CURRENT_POLY - 1].getCenter().getY()));
    //Scale
    for (int step = 0; step < Poly[CURRENT_POLY - 1].getVertSize(); step++)
    {
        float newX = 0, newY = 0, finalX, finalY;
        newX = Poly[CURRENT_POLY - 1].getVert(step).getX(); //Get X
        newY = Poly[CURRENT_POLY - 1].getVert(step).getY(); //Get Y
        finalX = cos(angle * D_TO_R)*newX - sin(angle * D_TO_R)*newY; //FinalX = cos(theta)*x - sin(theta)*y
        finalY = sin(angle * D_TO_R)*newX + cos(angle * D_TO_R)*newY; //FinalY = sin(theta)*x + cos(theta)*x
        Poly[CURRENT_POLY - 1].setVert(step, finalX, finalY);
    }
    //Translate back by centroid
    translate(Poly[CURRENT_POLY - 1].getCenter().getX(), Poly[CURRENT_POLY - 1].getCenter().getY());
}

void scale(float alpha, float beta)
{
    /*
        scale w.r.t center
        Using a homogeneous scaling matrix of   (alpha 0    | 0)
                                                (0     beta | 0)
                                                (--------------)
                                                (0     0    | 1)
        Apply the matrix to each point (Scaling by origin)
        For example, a polygon with vertices (0, 0) (0, 1) (1, 1) (1, 0) when applied to the scaling factor would then be:
        (0, 0) -> (0, 0)
        (1, 0) -> (2, 0)
        (1, 1) -> (2, 2)
        (1, 0) -> (2, 0)
        What we see above is not exactly what we want.
        We must move the center (centroid) of the polygon to the origin | centroid is calculated by taking the avg of all vertices
   ---- Three operations to scale by centroid:
            1. Translation (By -(centroid) )
            2. Scale (By scale matrix in every direction) - after translation, scaling factor will be applied to every point
            3. Translation (By (centroid) )
    */
    //First translate by -centroid
    translate(-(Poly[CURRENT_POLY - 1].getCenter().getX()), -(Poly[CURRENT_POLY - 1].getCenter().getY()));  
    //Scale
    for (int step = 0; step < Poly[CURRENT_POLY - 1].getVertSize(); step++)
    {
        float newX = 0, newY = 0;
        newX += Poly[CURRENT_POLY - 1].getVert(step).getX(); newX *= alpha; //X * alpha
        newY += Poly[CURRENT_POLY - 1].getVert(step).getY(); newY *= beta;  //Y * beta
        Poly[CURRENT_POLY - 1].setVert(step, newX, newY);
    }
    //Translate back by centroid
    translate(Poly[CURRENT_POLY - 1].getCenter().getX(), Poly[CURRENT_POLY - 1].getCenter().getY());
}


void clip()
{
    /*
        Clipping:
        Clipping can be done in two ways defined by us:
            1. Moving the viewport will change the polygon, so when window is moved back the polygon that was clipped is gone
            2. Moving the viewport will not change the polygon, so when the window is moved back the polygon is still there - can be done via temp

         So if its not too hard, whenever we clip a Poly, create a temp so the original is not changed
    */

    //Idk what i'm doing
    //I can't scan line
    //halp

    //This is gun be janky

    //Default values for testing
    X_0 = -20; X_1 =  20; X_2 = -20; X_3 = 20;
    Y_0 = -20; Y_1 = -20; Y_2 =  20; Y_3 = 20;
    //BRUTE FORCE BABY
    for (int yStep = 0; yStep < HEIGHT; yStep++)
    {
        if (yStep - MID_Y <= X_0)    //This is a comment...
        {
            lineDDA(-MID_X, yStep - MID_Y, MID_X, yStep - MID_Y, 1.0f, 1.0, 1.0f);
        }
        else if ((yStep - MID_Y) > Y_0 && (yStep - MID_Y) < Y_2)
        {
            lineDDA(-MID_X, yStep - MID_Y, X_0, yStep - MID_Y, 1.0f, 1.0f, 1.0f);
            lineDDA(X_1, yStep - MID_Y, MID_X, yStep - MID_Y, 1.0f, 1.0f, 1.0f);
        }
        else if (yStep - MID_Y >= Y_2)
        {
            lineDDA(-MID_X, yStep - MID_Y, MID_X, yStep - MID_Y, 1.0f, 1.0f, 1.0f);
        }

        //for (int xStep = 0; xStep < WIDTH; xStep++)
        //{
        //    if (yStep - MID_Y <= y0)    //Account for the MID's below 
        //    {
        //        //setPixel(xStep - MID_X, yStep - MID_Y, 1.0f, 1.0f, 1.0f);   //1.0f for white
        //    }
        //    else if (yStep - MID_Y > y0 && yStep - MID_Y < y2)
        //    {
        //        if (xStep - MID_X < x0 && xStep - MID_X > x3)
        //        {
        //            //setPixel(xStep - MID_X, yStep - MID_Y, 1.0f, 1.0f, 1.0f);
        //        }
        //    }
        //    else if (yStep - MID_Y >= y2)
        //    {
        //       // setPixel(xStep - MID_X, yStep - MID_Y, 1.0f, 1.0f, 1.0f);
        //    }
        //    else
        //        ;//Do nothing
        //}
    }

   // drawAxis(); //Reestablish deleted axis
    
}

void clipLoad()
{
    ifstream inFile;
    inFile.open(clipData);
    string line;
    //Janky ass hard coding going on here
    inFile >> X_0; inFile >> Y_0;
    inFile >> X_1; inFile >> Y_1;
    inFile >> X_2; inFile >> Y_2;
    inFile >> X_3; inFile >> Y_3;

    inFile.close();
}

//I hate parsing
void loadFile()
{
    //Files will have very strict formatting. Will crash unless its very specifically formatted
    ifstream inFile;    
    inFile.open(fileName); 
    string line;
    int totalVerts = 0, x, y;
    inFile >> WIDTH;    //Read Width
    MID_X = WIDTH / 2;
    inFile >> HEIGHT;   //Read Height
    MID_Y = HEIGHT / 2;
    inFile >> totalPolys;   //Read total Polys
    if (totalPolys <= maxPolys && totalPolys >= 0)
    {
        for (int step = 0; step < totalPolys; step++)
        {
            polygon newPoly(step);
            inFile >> totalVerts;
            for (int vertStep = 0; vertStep < totalVerts; vertStep++)
            {
                inFile >> x; inFile >> y;
                newPoly.addVert(x, y);
            }
            Poly.push_back(newPoly);
            Poly[step].populateEdges();
            Poly[step].getCentroid();
        }
    }
    else
        cout << "Too many or too few polygons in file. Please double check.\nShould be between 0 and 6.\n";
    inFile.close();
    //Everything should be set just as if user was entering info into the console
}

void saveFile()
{
    ofstream outFile;   
    outFile.open(fileName, ios::trunc); 
    outFile << WIDTH; outFile << " ";
    outFile << HEIGHT; outFile << endl;
    outFile << totalPolys << endl;
    for (int step = 0; step < totalPolys; step++)
    {
        outFile << Poly[step].getVertSize() << endl;
        for (int Vstep = 0; Vstep < Poly[step].getVertSize(); Vstep++)
        {
            outFile << Poly[step].getVert(Vstep).getX() << " " << Poly[step].getVert(Vstep).getY() << endl;
        }
    }

    outFile.close();  // IO testing

    //Save to POLY.txt

    //Write Height
    //Write Width
    //Write Polygons:
        //Write ID
        //Write Total Verts
        //Write Total Edges
        //Write Vertice and Edge Vectors
}

/*
    Format File as Follows:
        Window Width Height
        Number of Polygons
        Number of vertices in polygon
            X Y
            X Y
            X Y
        Number of vertices in polygon
            X Y
            X Y
            .......
*/
