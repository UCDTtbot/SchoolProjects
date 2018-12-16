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
#include "3DPolygon.h"

using namespace std;

float *PIXEL_BUFFER_MAIN; // [width * height * RGB] | [x * 3 + y * width * 3] for 0, then add 1 and then 2 
float *PIXEL_BUFFER_XY;
float *PIXEL_BUFFER_XZ;
float *PIXEL_BUFFER_YZ;
static int CURRENT_POLY = 0;    //Poly ID
static int MainWindow;  //Global for holding the mainWindow ID
static int subWindow_1, subWindow_2, subWindow_3;
static int WIDTH = 800, HEIGHT = 600;
static int SUB_WIDTH = WIDTH / 2, SUB_HEIGHT = HEIGHT / 2;
static int subWidth_1 = SUB_WIDTH, subHeight_1 = SUB_HEIGHT;
static int subWidth_2 = SUB_WIDTH, subHeight_2 = SUB_HEIGHT;
static int subWidth_3 = SUB_WIDTH, subHeight_3 = SUB_HEIGHT;
static int MID_X = WIDTH / 2, MID_Y = HEIGHT / 2; //EXTERNED VARIABLES FROM WINDOW.H
static int MastMax = 0, MastMin = 0;

//RGB to play with for lines
float RGB_CONV = 1 / (float)255;
float R = 0.0f;
float G = 0.0f;
float B = 255.0f * RGB_CONV;

//NDC
static vector<pair<TPnt, TPnt> > NDC_XY; //Vector of 3D Pnts, one for each Polygon
static vector<pair<TPnt, TPnt> > NDC_XZ;
static vector<pair<TPnt, TPnt> > NDC_YZ;
static Edge rotateAxis;
static bool rotating = false;

static int mainBuf = 1, xyBuf = 2, xzBuf = 3, yzBuf = 4;

int drawAlg = 1; //Default drawAlg to DDA | 2 = Bres
//Menu Vairables
static int menuID, subMenuID1, subMenuID2;  //Globals for holding menu IDs
static int menuValue;   //Menu item selected
                        //File I/O variables
string fileName = "POLYS.dat";  //Default filename. Possibly will not change
ofstream outputFile; //output file object
ifstream inputFile; //input file object
void loadFile();    //Function for loading from file
void saveFile();    //Function for saving to file


const float  PI_F = 3.14159265358979f; //PI for degrees to radians
const float D_TO_R = PI_F / 180;    //Use to convert degrees_to_radian
                                    
static int totalPolys = 0;  //Total polys made
static int maxPolys = 3;    //Max number of polys allowed to be made in this program
vector<THPolygon> Poly;   //Vector for holding polys that have been made
                        //Functions for setting up OpenGL
void main_display(); //OpenGL Display function
void sub_1_display();   //XY
void sub_2_display();   //XZ
void sub_3_display();   //YZ

void init();    //function to initiate many things in my program
                //For Menu
void createMenu();  //function to create the menus
void menu(int num); //OpenGL menu callback function

                    //Line and Polygon Functions
void setPixel(int buffer, float x, float y, float R, float G, float B); //Function to set pixels in pixel buffer. Takes it coordinate and RGB color value
void lineDDA(int buffer, float X0, float Y0, float X, float Y, float R, float G, float B);  //Draws line from initial point to final point. Also takes RGB color
void lineBres(int buffer, int X0, int Y0, int X_End, int Y_End, float R, float G, float B); //See above
void consoleMenu(); //Function for setting up the console menu
void drawLines();   //Function that draws lines to pixel buffer using either lineDDA or lineBres
//
void translate(float x, float y, float z);   //Translation func
void rotate(float angle);       //Rotation func
void scale(float alpha, float beta, float zeta);    //Scaling func
//
void drawAxis();
void redisplayAllWindows();
//
void getMasters();
void normalize(int, int, THPolygon);
void normalizeRotation(TPnt, TPnt);
//
void displayVerts(int PolyID);
//
void getFileName();
//
void selectPoly(int selection);
//
void animate(int value);

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
    getFileName();
    //Prep file I/O
    loadFile();

    //Window setup below
    glutInit(&argc, argv);
    //TODO: Setup subWindows
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowPosition(250, 100);
    glutInitWindowSize(WIDTH, HEIGHT);

    MainWindow = glutCreateWindow("Project 2");
    glutDisplayFunc(main_display);

    subWindow_1 = glutCreateSubWindow(MainWindow, 0, 0, subWidth_1, subHeight_1);  //XY        
    glutDisplayFunc(sub_1_display);

    subWindow_2 = glutCreateSubWindow(MainWindow, MID_X, 0, subWidth_2, subHeight_2);  //XZ 
    glutDisplayFunc(sub_2_display);

    subWindow_3 = glutCreateSubWindow(MainWindow, 0, MID_Y, subWidth_3, subHeight_3);  //YZ 
    glutDisplayFunc(sub_3_display);

    createMenu();//Initialize Menu and Window stuffs
    init();

    //Starts the main GLUT loop
    createMenu();
    displayVerts(CURRENT_POLY);
    glutMainLoop();

    return 0;
}
//Initialize window color and PixelBuffer
void init()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

    PIXEL_BUFFER_MAIN = new float[WIDTH * HEIGHT * 3];
    PIXEL_BUFFER_XY = new float[subWidth_1 * subHeight_1 * 3];
    PIXEL_BUFFER_XZ = new float[subWidth_2 * subHeight_2 * 3];
    PIXEL_BUFFER_YZ = new float[subWidth_3 * subHeight_3 * 3];

    for (int step = 0; step < WIDTH * HEIGHT * 3; step++)//Set initial buffer to a blank white backround
        PIXEL_BUFFER_MAIN[step] = 1.0f;
    for (int step = 0; step < subWidth_1 * subHeight_1 * 3; step++)             //xy
        PIXEL_BUFFER_XY[step] = 0.8f;
    for (int step = 0; step < subWidth_2 * subHeight_2 * 3; step++)             //xz
        PIXEL_BUFFER_XZ[step] = 0.6f;
    for (int step = 0; step < subWidth_3 * subHeight_3 * 3; step++)             //yz
        PIXEL_BUFFER_YZ[step] = 0.4f;
    
    for (int step = 0; step < totalPolys; step++)
    {
        pair<TPnt, TPnt> *newPnt_XY = new pair<TPnt, TPnt>; newPnt_XY->first.z = -1; newPnt_XY->second.z = -1;
        pair<TPnt, TPnt> *newPnt_XZ = new pair<TPnt, TPnt>; newPnt_XZ->first.y = -1; newPnt_XZ->second.y = -1;
        pair<TPnt, TPnt> *newPnt_YZ = new pair<TPnt, TPnt>; newPnt_YZ->first.x = -1; newPnt_YZ->second.x = -1;

        NDC_XY.push_back(*newPnt_XY);
        NDC_XZ.push_back(*newPnt_XZ);
        NDC_YZ.push_back(*newPnt_YZ);
    }

    drawLines();

//    drawAxis();
    glutSetWindow(MainWindow);

    //drawAxis();
    //testing VS pushing


}

void drawAxis()
{
    //Main Window
    lineDDA(mainBuf, 0, MID_Y, WIDTH, MID_Y, 0.0f, 0.0f, 0.0f); //Setup horizontal axis
    lineDDA(mainBuf, MID_X, 0, MID_X, HEIGHT - 1, 0.0f, 0.0f, 0.0f); //Setup vertical axis

    lineDDA(xyBuf, 0, 0, 200, 200, 0.0f, 0.0f, 0.0f);
    lineDDA(xzBuf, 0, 0, 200, 200, 0.0f, 0.0f, 0.0f);
    lineDDA(yzBuf, 0, 0, 200, 200, 0.0f, 0.0f, 0.0f);
}

void setPixel(int Buffer, float x, float y, float R, float G, float B)
{   //Backround is Init to 1.0f = white, set pixel RGB to 0.0f for black

    //Main = 0 | XY = 1 | XZ = 2 | YZ = 3
    if (Buffer == mainBuf) //Main
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                PIXEL_BUFFER_MAIN[(((int)round(x)) * 3 + ((int)round(y)) * WIDTH * 3) + step] = R;  //Set R
            else if (step == 1)
                PIXEL_BUFFER_MAIN[(((int)round(x)) * 3 + ((int)round(y)) * WIDTH * 3) + step] = G;  //Set G
            else if (step == 2)
                PIXEL_BUFFER_MAIN[(((int)round(x)) * 3 + ((int)round(y)) * WIDTH * 3) + step] = B;  //Set B
        }
    }
    else if (Buffer == xyBuf) //XY
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                PIXEL_BUFFER_XY[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_1 * 3) + step] = R;  //Set R
            else if (step == 1)
                PIXEL_BUFFER_XY[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_1 * 3) + step] = G;  //Set G
            else if (step == 2)
                PIXEL_BUFFER_XY[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_1 * 3) + step] = B;  //Set B
        }
    }
    else if (Buffer == xzBuf) //XZ
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                PIXEL_BUFFER_XZ[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_2 * 3) + step] = R;  //Set R
            else if (step == 1)
                PIXEL_BUFFER_XZ[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_2 * 3) + step] = G;  //Set G
            else if (step == 2)
                PIXEL_BUFFER_XZ[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_2 * 3) + step] = B;  //Set B
        }
    }
    else if (Buffer == yzBuf) //YZ
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                PIXEL_BUFFER_YZ[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_3 * 3) + step] = R;  //Set R
            else if (step == 1)
                PIXEL_BUFFER_YZ[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_3 * 3) + step] = G;  //Set G
            else if (step == 2)
                PIXEL_BUFFER_YZ[(((int)round(x)) * 3 + ((int)round(y)) * subWidth_3 * 3) + step] = B;  //Set B
        }
    }
    else
        ;
}

//Parameters of lineDDA are the beginning and end points of our wanted line.
void lineDDA(int buffer, float X0, float Y0, float X_End, float Y_End, float R, float G, float B)
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
        setPixel(buffer, round(Xn), round(Yn), R, G, B); //Set init point
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc;
            setPixel(buffer, round(Xn), round(Yn), R, G, B);
        }//(100, 150) (0, 150)
    }
    else //Slope is defined
    {
        if (fabs(delta_X) > fabs(delta_Y)) //number of steps to take in the for loop (the delta) 
            steps = fabs(delta_X);   //This if determines 0 < m < 1 or 1 < m < inf
        else
            steps = round(fabs(delta_Y));
        xInc = float(delta_X) / float(steps); //will set to either 1 or m, for the equation for Xn+1 = x + 1/m or Yn+1 = y+1/m and so forth
        yInc = float(delta_Y) / float(steps);

        setPixel(buffer, round(Xn), round(Yn), R, G, B);//Creates initial pixel
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc; //Increment X and Y by their respective values
            Yn += yInc;
            setPixel(buffer, round(Xn), round(Yn), R, G, B);
        }
    }
}

void lineBres(int buffer, int X0, int Y0, int X_End, int Y_End, float R, float G, float B)
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

    if (delta_X == 0)    //Case for vertical line
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

        setPixel(buffer, Xn, Yn, R, G, B);
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
            setPixel(buffer, Xn, Yn, R, G, B);
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
        setPixel(buffer, Xn, Yn, R, G, B);
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
            setPixel(buffer, Xn, Yn, R, G, B);
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
        setPixel(buffer, Xn, Yn, R, G, B);
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
            setPixel(buffer, Xn, Yn, R, G, B);
        }
    }
}

void main_display()
{
    //Function that is looped through
    glutSetWindow(MainWindow);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    //Draws the PixelBuffer to screen - reword for subwindows
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, PIXEL_BUFFER_MAIN);

    glFlush();
}

void sub_1_display() //xy
{
    glutSetWindow(subWindow_1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    //Draws the PixelBuffer to screen - reword for subwindows
    glDrawPixels(subWidth_1, subHeight_1, GL_RGB, GL_FLOAT, PIXEL_BUFFER_XY);

    glFlush();
}

void sub_2_display()    //xz

{
    glutSetWindow(subWindow_2);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    //Draws the PixelBuffer to screen - reword for subwindows
    glDrawPixels(subWidth_2, subHeight_2, GL_RGB, GL_FLOAT, PIXEL_BUFFER_XZ);

    glFlush();
}

void sub_3_display()    //yz
{
    glutSetWindow(subWindow_3);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    //Draws the PixelBuffer to screen - reword for subwindows
    glDrawPixels(subWidth_3, subHeight_3, GL_RGB, GL_FLOAT, PIXEL_BUFFER_YZ);

    glFlush();
}


//Menus will be used for  manipulating (scaling, translation, and rotation)
//Menu will be able to select a polygon
void createMenu()
{
    //First create submenu
    glutSetWindow(MainWindow);
    subMenuID1 = glutCreateMenu(menu);  //SubMenu for Polygon IDs
    glutAddMenuEntry("Polygon #0", 1);
    glutAddMenuEntry("Polygon #1", 2);
    glutAddMenuEntry("Polygon #2", 3);
    subMenuID2 = glutCreateMenu(menu);  //SubMenu for Manipulate
    glutAddMenuEntry("Translate", 4);
    glutAddMenuEntry("Scale", 5);
    glutAddMenuEntry("Rotate", 6);
    menuID = glutCreateMenu(menu);  //Setup main Menu
    glutAddSubMenu("Select Polygon", subMenuID1);
    glutAddSubMenu("Manipulate", subMenuID2);   //Attaches to subMenu 2
    glutAddMenuEntry("Display Vertices and Edges", 9);
    glutAddMenuEntry("Delete Rotation Axis", 10);
    glutAddMenuEntry("Animate", 11);
    glutAddMenuEntry("Save Polygons", 7);
    glutAddMenuEntry("Load Polygons", 8);
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
    {
        if (totalPolys < 1)
            cout << "Polygon does not exist.";
        else
        {
            CURRENT_POLY = 0;
            cout << "Selected Polygon " << CURRENT_POLY << endl;
            displayVerts(CURRENT_POLY);
        }
    }
    else if (num == 2)  //Polygon #2
    {
        if (totalPolys < 2)
            cout << "Polygon does not exist.";
        else
        {
            CURRENT_POLY = 1;
            cout << "Selected Polygon " << CURRENT_POLY << endl;
            displayVerts(CURRENT_POLY);
        }
    }
    else if (num == 3)  //Polygon #3
    {
        if (totalPolys < 3)
            cout << "Polygon does not exist.";
        else
        {
            CURRENT_POLY = 2;
            cout << "Selected Polygon " << CURRENT_POLY << endl;
            displayVerts(CURRENT_POLY);
        }
    }
    else if (num == 4)  //Translation
    {
        int x = 0, y = 0, z = 0;
        cout << "Please enter the translation vector: \nX: "; cin >> x;
        cout << "Y: "; cin >> y;
        cout << "Z: "; cin >> z;
        cout << "Translating and redrawing....\nReturn to GLUT Window\n";
        translate(x, y, z);
    }
    else if (num == 5)  //Scale
    {
        float x, y, z;
        cout << "Please enter the scale vector: \nAlpha: "; cin >> x;
        cout << "Beta: "; cin >> y;
        cout << "Zeta: "; cin >> z;
        cout << "Scaling and redrawing....\nReturn to GLUT window\n";
        scale(x, y, z);
    }
    else if (num == 6)  //Rotation
    {
        int angle, x1, y1, z1, x2, y2, z2;
        cout << "Please enter rotational axis as two points (X, Y, Z): \n";
        cout << "First X: "; cin >> x1;
        cout << "First Y: "; cin >> y1;
        cout << "First Z: "; cin >> z1;
        cout << "Second X: "; cin >> x2;
        cout << "Second Y: "; cin >> y2;
        cout << "Second Z: "; cin >> z2;
        cout << "Please enter an angle in degrees: \nAlpha: "; cin >> angle;
        cout << "Rotating by " << angle << " around axis (" << x1 << ", " << y1 << ", " << z1 << ")-(" << x2 << ", " << y2 << ", " << z2 << ")\n";
        rotateAxis.first.x = x1; rotateAxis.first.y = y1; rotateAxis.first.z = z1;
        rotateAxis.second.x = x2; rotateAxis.second.y = y2; rotateAxis.second.z = z2;
        rotating = true;
        rotate(angle);
    }
    else if (num == 7)  //Save
        saveFile();
    else if (num == 8)  //Load
    {
        Poly.clear();
        loadFile();
        init();
    }
    else if (num == 9)
    {
        displayVerts(CURRENT_POLY);
    }
    else if (num == 10)
    {
        rotating = false;
        drawLines();
    }
    else if (num == 11)
    {
        animate(1);
    }
    else if (num == 12)
    {
        animate(0);
    }
    else //Fallback function
    {
        menuValue = num;
    }

    redisplayAllWindows();  //One call to postRedisplay all windows

}

void redisplayAllWindows()
{
    glutSetWindow(MainWindow);
    glutPostRedisplay(); //Repost the display function to redraw everything
    glutSetWindow(subWindow_1);
    glutPostRedisplay();
    glutSetWindow(subWindow_2);
    glutPostRedisplay();
    glutSetWindow(subWindow_3);
    glutPostRedisplay();
    glutSetWindow(MainWindow);

}

void consoleMenu()
{
    float x = 0, y = 0;
    int decision = -1, totalVerts = 0, curID = 0;
    bool isDone = false;
    //Console Window setup
    cout << "Polylines will be drawn in order of given vertices.\n";

    while (totalPolys < maxPolys && isDone == false)
    {
        cout << "You can create " << maxPolys - totalPolys << " more polygons.\n";
        cout << "Would you like to create:\n";
        cout << "1. Polygon\n3. Finished. Draw Polygons\n"; cin >> decision;

        while (decision < 0 || decision > 3)
        {
            cout << "Invalid input\n";
            cout << "You can create " << maxPolys - totalPolys << " more polygons.\n";
            cout << "Would you like to create:\n";
            cout << "1. Polygon\n3. Finished. Draw Polygons\n"; cin >> decision;
        }
        if (decision == 1)
        {
            //Create Polygon
            curID = totalPolys;
            THPolygon newPoly(curID);
            
            cout << "Must be reworked. ";

            Poly.push_back(newPoly);
            totalPolys++;
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

void drawLines()  //drawLine for buffer
{
    //Reset Buffers
    for (int step = 0; step < WIDTH * HEIGHT * 3; step++)//Set initial buffer to a blank white backround
        PIXEL_BUFFER_MAIN[step] = 1.0f;
    for (int step = 0; step < subWidth_1 * subHeight_1 * 3; step++)             //xy
        PIXEL_BUFFER_XY[step] = 0.8f;
    for (int step = 0; step < subWidth_2 * subHeight_2 * 3; step++)             //xz
        PIXEL_BUFFER_XZ[step] = 0.6f;
    for (int step = 0; step < subWidth_3 * subHeight_3 * 3; step++)             //yz
        PIXEL_BUFFER_YZ[step] = 0.4f;

    for (int pStep = 0; pStep < totalPolys; pStep++)   //For Each Poly and For each edge IN that poly
    {
        THPolygon curPoly = Poly[pStep];

        if (drawAlg == 2) // do Bres
        {
            for (int eStep = 0; eStep < curPoly.totalEdges; eStep++)
            {
                normalize(pStep, eStep, curPoly);
                lineBres(xyBuf, NDC_XY[pStep].first.x, NDC_XY[pStep].first.y, NDC_XY[pStep].second.x, NDC_XY[pStep].second.y, 1.0f, 0.0f, 0.0f);
                lineBres(xzBuf, NDC_XZ[pStep].first.x, NDC_XZ[pStep].first.z, NDC_XZ[pStep].second.x, NDC_XZ[pStep].second.z, 0.0f, 1.0f, 0.0f);
                lineBres(yzBuf, NDC_YZ[pStep].first.y, NDC_YZ[pStep].first.z, NDC_YZ[pStep].second.y, NDC_YZ[pStep].second.z, 0.0f, 0.0f, 1.0f);
            }

        }
        else //Default to DDA
        {
            for (int eStep = 0; eStep < curPoly.totalEdges; eStep++)
            {
                normalize(pStep, eStep, curPoly);
                lineDDA(xyBuf, NDC_XY[pStep].first.x, NDC_XY[pStep].first.y, NDC_XY[pStep].second.x, NDC_XY[pStep].second.y, 1.0f, 0.0f, 0.0f);
                lineDDA(xzBuf, NDC_XZ[pStep].first.x, NDC_XZ[pStep].first.z, NDC_XZ[pStep].second.x, NDC_XZ[pStep].second.z, 0.0f, 1.0f, 0.0f);
                lineDDA(yzBuf, NDC_YZ[pStep].first.y, NDC_YZ[pStep].first.z, NDC_YZ[pStep].second.y, NDC_YZ[pStep].second.z, 0.0f, 0.0f, 1.0f);
            }
        }
    }

    if (rotating == false)
    {
        TPnt Pnt1 = rotateAxis.first, Pnt2 = rotateAxis.second;
        normalizeRotation(Pnt1, Pnt2);
    }
    else if (rotating == true)
    {
        //Draw rotational axis
        TPnt Pnt1 = rotateAxis.first, Pnt2 = rotateAxis.second;
        normalizeRotation(Pnt1, Pnt2);

        rotating = false;
    }
}

void normalize(int pStep, int eStep, THPolygon curPoly)
{
    getMasters();
    float xy_1X, xy_1Y, xy_2X, xy_2Y;
    float xz_1X, xz_1Z, xz_2X, xz_2Z;
    float yz_1Y, yz_1Z, yz_2Y, yz_2Z;
    float masterMax = MastMax, masterMin = MastMin;
    //(point - masterMin) / (masterMax - masterMin)
    xy_1X = ((curPoly.edges[eStep].first.x - masterMin) / (float)(masterMax - masterMin));
    xy_1Y = ((curPoly.edges[eStep].first.y - masterMin) / (float)(masterMax - masterMin));
    xy_2X = ((curPoly.edges[eStep].second.x - masterMin) / (float)(masterMax - masterMin));
    xy_2Y = ((curPoly.edges[eStep].second.y - masterMin) / (float)(masterMax - masterMin));

    xz_1X = ((curPoly.edges[eStep].first.x - masterMin) / (float)(masterMax - masterMin));
    xz_1Z = ((curPoly.edges[eStep].first.z - masterMin) / (float)(masterMax - masterMin));
    xz_2X = ((curPoly.edges[eStep].second.x - masterMin) / (float)(masterMax - masterMin));
    xz_2Z = ((curPoly.edges[eStep].second.z - masterMin) / (float)(masterMax - masterMin));

    yz_1Y = ((curPoly.edges[eStep].first.y - masterMin) / (float)(masterMax - masterMin));
    yz_1Z = ((curPoly.edges[eStep].first.z - masterMin) / (float)(masterMax - masterMin));
    yz_2Y = ((curPoly.edges[eStep].second.y - masterMin) / (float)(masterMax - masterMin));
    yz_2Z = ((curPoly.edges[eStep].second.z - masterMin) / (float)(masterMax - masterMin));

    //cout << "Printing out Normalized Coords. MasterMin: " << masterMin << " MasterMax: " << masterMax << "\n";
    //cout << "XY, edge #" << eStep << ":\n";
    //cout << "First: (" << xy_1X << ", " << xy_1Y << ") Second: (" << xy_2X << ", " << xy_2Y << ") || " << "First: (" << xy_1X * subWidth_1 << ", " << xy_1Y * (subHeight_1 - 1) << ") Second : (" << xy_2X * subWidth_1 << ", " << xy_2Y * (subHeight_1 - 1) << ") \n";

    //cout << "XZ, edge #" << eStep << ":\n";
    //cout << "First: (" << xz_1X << ", " << xz_1Z << ") Second: (" << xz_2X << ", " << xz_2Z << ")\n";

    //cout << "YZ, edge #" << eStep << ":\n";
    //cout << "First: (" << yz_1Y << ", " << yz_1Z << ") Second: (" << yz_2Y << ", " << yz_2Z << ")\n";

    NDC_XY[pStep].first.x = xy_1X * (subWidth_1 / 2); NDC_XY[pStep].first.y = xy_1Y * ((subHeight_1 - 1)/2);
    NDC_XY[pStep].second.x = xy_2X * (subWidth_1 / 2); NDC_XY[pStep].second.y = xy_2Y * ((subHeight_1 - 1)/2);

    NDC_XZ[pStep].first.x = xz_1X * (subWidth_2 / 2); NDC_XZ[pStep].first.z = xz_1Z * ((subHeight_2 - 1) / 2);
    NDC_XZ[pStep].second.x = xz_2X * (subWidth_2 / 2); NDC_XZ[pStep].second.z = xz_2Z * ((subHeight_2 - 1) / 2);

    NDC_YZ[pStep].first.y = yz_1Y * (subWidth_3 / 2); NDC_YZ[pStep].first.z = yz_1Z * ((subHeight_3 - 1) / 2);
    NDC_YZ[pStep].second.y = yz_2Y * (subWidth_3 / 2); NDC_YZ[pStep].second.z = yz_2Z * ((subHeight_3 - 1) / 2);
}

void normalizeRotation(TPnt Pnt1, TPnt Pnt2)
{
    getMasters();
    float xy_1X, xy_1Y, xy_2X, xy_2Y;
    float xz_1X, xz_1Z, xz_2X, xz_2Z;
    float yz_1Y, yz_1Z, yz_2Y, yz_2Z;
    float masterMax = MastMax, masterMin = MastMin;
    //(point - masterMin) / (masterMax - masterMin)
    xy_1X = (((Pnt1.x - masterMin) / (float)(masterMax - masterMin)) * (subWidth_1 / 2));
    xy_1Y = (((Pnt1.y - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_1 - 1) / 2));
    xy_2X = (((Pnt2.x - masterMin) / (float)(masterMax - masterMin)) * (subWidth_1 / 2));
    xy_2Y = (((Pnt2.y - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_1 - 1) / 2));

    xz_1X = (((Pnt1.x - masterMin) / (float)(masterMax - masterMin)) * (subWidth_2 / 2));
    xz_1Z = (((Pnt1.z - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_2 - 1) / 2));
    xz_2X = (((Pnt2.x - masterMin) / (float)(masterMax - masterMin)) * (subWidth_2 / 2));
    xz_2Z = (((Pnt2.z - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_2 - 1) / 2));

    yz_1Y = (((Pnt1.y - masterMin) / (float)(masterMax - masterMin)) * (subWidth_3 / 2));
    yz_1Z = (((Pnt1.z - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_3 - 1) / 2));
    yz_2Y = (((Pnt2.y - masterMin) / (float)(masterMax - masterMin)) * (subWidth_3 / 2));
    yz_2Z = (((Pnt2.z - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_3 - 1) / 2));

    if (rotating == false)
    {
        lineDDA(xyBuf, xy_1X, xy_1Y, xy_2X, xy_2Y, 0.8f, 0.8f, 0.8f);
        lineDDA(xzBuf, xz_1X, xz_1Z, xz_2X, xz_2Z, 0.6f, 0.6f, 0.6f);
        lineDDA(yzBuf, yz_1Y, yz_1Z, yz_2Y, yz_2Z, 0.4f, 0.4f, 0.4f);
    }
    else
    {
        lineDDA(xyBuf, xy_1X, xy_1Y, xy_2X, xy_2Y, 0.0f, 0.0f, 0.0f);
        lineDDA(xzBuf, xz_1X, xz_1Z, xz_2X, xz_2Z, 0.0f, 0.0f, 0.0f);
        lineDDA(yzBuf, yz_1Y, yz_1Z, yz_2Y, yz_2Z, 0.0f, 0.0f, 0.0f);
    }
}

void translate(float xIn, float yIn, float zIn)
{
    //Move every point of the polygon by X and Y
    //Does not support going under (0,0)
    THPolygon curPoly = Poly[CURRENT_POLY];
    float newX = 0, newY = 0, newZ = 0;   //New x, y, z
    for (int step = 0; step < curPoly.totalEdges; step++)
    {
        newX = curPoly.edges[step].first.x + xIn;   curPoly.edges[step].first.x = newX;
        newY = curPoly.edges[step].first.y + yIn;   curPoly.edges[step].first.y = newY;
        newZ = curPoly.edges[step].first.z + zIn;   curPoly.edges[step].first.z = newZ;
//        cout << "First, X: " << newX << " Y: " << newY << " Z: " << newZ << endl;
        newX = curPoly.edges[step].second.x + xIn;  curPoly.edges[step].second.x = newX;
        newY = curPoly.edges[step].second.y + yIn;  curPoly.edges[step].second.y = newY;
        newZ = curPoly.edges[step].second.z + zIn;  curPoly.edges[step].second.z = newZ;
//        cout << "Second, X: " << newX << " Y: " << newY << " Z: " << newZ << endl;
    }
    for (int vStep = 0; vStep < curPoly.totalVerts; vStep++)
    {
        newX = curPoly.vertices[vStep].x + xIn;
        newY = curPoly.vertices[vStep].y + yIn;
        newZ = curPoly.vertices[vStep].z + zIn;
        curPoly.setVert(vStep, newX, newY, newZ);
    }
    Poly[CURRENT_POLY] = curPoly;
    drawLines();
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
    THPolygon curPoly = Poly[CURRENT_POLY];
    TPnt Pnt1 = rotateAxis.first;
    TPnt Pnt2 = rotateAxis.second;
    float dx = Pnt2.x - Pnt1.x;
    float dy = Pnt2.y - Pnt1.y;
    float dz = Pnt2.z - Pnt1.z;
    float I = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
    float ux = dx / (float)I;
    float uy = dy / (float)I;
    float uz = dz / (float)I;
    float newX = 0, newY = 0, newZ = 0;
    float x, y, z;
    float cosA = cos(angle*D_TO_R), sinA = sin(angle*D_TO_R);
    float oneC = (1 - cosA);
    float mat1, mat2, mat3, mat4, mat5, mat6, mat7, mat8, mat9;
    mat1 = ux*ux*oneC + cosA;           //These correspond to the matrix found in the book, where it is a 3x3 matrix. mat1 is top left, mat2 is top middle, mat9 is bottom right
    mat2 = ux*uy*oneC - (uz*sinA);
    mat3 = ux*uz*oneC + (uy*sinA);
    mat4 = uy*ux*oneC + (uz*sinA);
    mat5 = uy*uy*oneC + cosA;
    mat6 = uy*uz*oneC - (ux*sinA);
    mat7 = uz*ux*oneC - (uy*sinA);
    mat8 = uz*uy*oneC + (ux*sinA);
    mat9 = uz*uz*oneC + cosA;
    //First translate by -centroid
    translate(-(curPoly.centroid.x), -(curPoly.centroid.y), -(Poly[CURRENT_POLY].centroid.z));
    //Rotate - Edges
    for (int step = 0; step < curPoly.totalEdges; step++)
    {
        //Edge Pnt1
        x = curPoly.edges[step].first.x;
        y = curPoly.edges[step].first.y;
        z = curPoly.edges[step].first.z;
        newX = x*mat1 + y*mat2 + z*mat3;
        newY = x*mat4 + y*mat5 + z*mat6;
        newZ = x*mat7 + y*mat8 + z*mat9;
        curPoly.edges[step].first.x = newX;
        curPoly.edges[step].first.y = newY;
        curPoly.edges[step].first.z = newZ;

        //Edge Pnt2
        x = curPoly.edges[step].second.x;
        y = curPoly.edges[step].second.y;
        z = curPoly.edges[step].second.z;
        newX = x*mat1 + y*mat2 + z*mat3;
        newY = x*mat4 + y*mat5 + z*mat6;
        newZ = x*mat7 + y*mat8 + z*mat9;
        curPoly.edges[step].second.x = newX;
        curPoly.edges[step].second.y = newY;
        curPoly.edges[step].second.z = newZ;

    }
    //Rotate - Verts
    for (int step = 0; step < curPoly.totalVerts; step++)
    {
        x = curPoly.vertices[step].x;
        y = curPoly.vertices[step].y;
        z = curPoly.vertices[step].z;
        newX = x*mat1 + y*mat2 + z*mat3;
        newY = x*mat4 + y*mat5 + z*mat6;
        newZ = x*mat7 + y*mat8 + z*mat9;
        curPoly.setVert(step, newX, newY, newZ);
    }
    //Translate back by centroid
    translate(curPoly.centroid.x, curPoly.centroid.y, curPoly.centroid.z);
    Poly[CURRENT_POLY] = curPoly;
    drawLines();
}

void scale(float alpha, float beta, float zeta)
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
    THPolygon curPoly = Poly[CURRENT_POLY];
    //First translate by -centroid
    translate(-(curPoly.centroid.x), -(curPoly.centroid.y), -(curPoly.centroid.z));
    //Scale - edges
    for (int step = 0; step < curPoly.totalEdges; step++)
    {
        float newX1 = 0, newY1 = 0, newZ1 = 0;
        float newX2 = 0, newY2 = 0, newZ2 = 0;
        newX1 += curPoly.edges[step].first.x; newX1 *= alpha;    curPoly.edges[step].first.x = newX1;   //New vert = X * alpha
        newY1 += curPoly.edges[step].first.y; newY1 *= beta;     curPoly.edges[step].first.y = newY1;   //New vert = Y * beta
        newZ1 += curPoly.edges[step].first.z; newZ1 *= zeta;     curPoly.edges[step].first.z = newZ1;   //New vert = Z * zeta

        newX2 += curPoly.edges[step].second.x; newX2 *= alpha;   curPoly.edges[step].second.x = newX2;
        newY2 += curPoly.edges[step].second.y; newY2 *= beta;    curPoly.edges[step].second.y = newY2;
        newZ2 += curPoly.edges[step].second.z; newZ2 *= zeta;    curPoly.edges[step].second.z = newZ2;
    }
    //Verts
    for (int vStep = 0; vStep < curPoly.totalVerts; vStep++)
    {
        float newX, newY, newZ;
        newX = curPoly.vertices[vStep].x * alpha;
        newY = curPoly.vertices[vStep].y * beta;
        newZ = curPoly.vertices[vStep].z * zeta;
        curPoly.setVert(vStep, newX, newY, newZ);
    }
    //Translate back by centroid
    translate(curPoly.centroid.x, curPoly.centroid.y, curPoly.centroid.z);
    Poly[CURRENT_POLY] = curPoly;
    drawLines();
}

void getMasters()
{
    //Normalize every vert from world coords to [0.0, 1.0]
    //Get MasterMax and MasterMin
    float x, y, z;
    THPolygon curPoly;
    for (int polyStep = 0; polyStep < totalPolys; polyStep++)
    {
        curPoly = Poly[polyStep];
        for (int step = 0; step < curPoly.totalVerts; step++)
        {
            x = curPoly.vertices[step].x;
            y = curPoly.vertices[step].y;
            z = curPoly.vertices[step].z;

            //MastMax
            if (x > MastMax)
                MastMax = x;
            if (y > MastMax)
                MastMax = y;
            if (z > MastMax)
                MastMax = z;

            //MastMin
            if (x < MastMin)
                MastMin = x;
            if (y < MastMin)
                MastMin = y;
            if (z < MastMin)
                MastMin = z;
        }
    }
    //Normalize a coordinate of x y by ( (x - mastMin) / (mastMax - mastMin), (y - mastMin) / (mastMax - mastMin) )
}

//I hate parsing

void loadFile()
{
    //Files will have very strict formatting. Will crash unless its very specifically formatted
    ifstream inFile;
    inFile.open(fileName.c_str());
    string line;
    int totalVerts = 0, totalEdges = 0;
    int pnt1, pnt2;
    float x, y, z;
    inFile >> totalPolys;   //Read total Polys
    if (totalPolys <= maxPolys && totalPolys >= 0)
    {
        for (int step = 0; step < totalPolys; step++)
        {
            THPolygon newPoly(step);
            inFile >> totalVerts;
            for (int vertStep = 0; vertStep < totalVerts; vertStep++)
            {
                inFile >> x; inFile >> y; inFile >> z;
                newPoly.addVert(x, y, z);
            }
            inFile >> totalEdges;
            for (int edgeStep = 0; edgeStep < totalEdges; edgeStep++)
            {
                inFile >> pnt1; inFile >> pnt2;
                newPoly.createEdge(newPoly.vertices[pnt1 - 1], newPoly.vertices[pnt2 - 1], pnt1, pnt2);
            }
            Poly.push_back(newPoly);
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
    outFile.open(fileName.c_str(), ios::trunc);
    outFile << totalPolys << endl;
    for (int step = 0; step < totalPolys; step++)
    {
        outFile << Poly[step].totalVerts << endl;
        for (int Vstep = 0; Vstep < Poly[step].totalVerts; Vstep++)
        {
            outFile << Poly[step].vertices[Vstep].x << " " << Poly[step].vertices[Vstep].y << " " << Poly[step].vertices[Vstep].z << endl;
        }
        outFile << Poly[step].totalEdges << endl;
        for (int Estep = 0; Estep < Poly[step].totalEdges; Estep++)
        {
            outFile << Poly[step].edges[Estep].fstCoord << " " << Poly[step].edges[Estep].scdCoord << endl;
        }
    }

    outFile.close();  // IO testing

                      //Save to POLY.txt

                      //Write Polygons:
                      //Write ID
                      //Write Total Verts
                      //Write Total Edges
                      //Write Vertice and Edge Vectors
}

void getFileName()
{
    cout << "Please enter file name. Enter d for default name (POLYS.dat): \n";
    cin >> fileName;
    if (fileName.compare("d") == 0|| fileName.compare("D") == 0)
        fileName = "POLYS.dat";
    cout << "Loaded file: " << fileName << endl;
    
}

void displayVerts(int PolyID)
{
    THPolygon curPoly = Poly[PolyID];
    
    cout << "Current Polygon: " << PolyID << endl;
    cout << "Vertices for polygon #" << PolyID << " are: \n";
    for (int vertStep = 0; vertStep < curPoly.totalVerts; vertStep++)
    {
        cout << curPoly.vertices[vertStep].x << " " << curPoly.vertices[vertStep].y << " " << curPoly.vertices[vertStep].z << "\n";
    }
    cout << "Defined edges for polygon #" << PolyID << " are:\n";
    for (int edgeStep = 0; edgeStep < curPoly.totalEdges; edgeStep++)
    {
       cout << curPoly.edges[edgeStep].fstCoord << " " << curPoly.edges[edgeStep].scdCoord << " \n";
    }
    
}

void animate(int value)
{
    if (value != 0)
    {
        rotateAxis.first.x = 0; rotateAxis.first.y = 0; rotateAxis.first.z = 0;
        rotateAxis.second.x = 500; rotateAxis.second.y = 500; rotateAxis.second.z = 500;
        rotate(15);
        glutTimerFunc(250, animate, 1);
        redisplayAllWindows();
    }
    else
    {
        glutTimerFunc(0, animate, 0);
        redisplayAllWindows();
    }
}
/*
    File Formatted as follows:
        #Of objects/polygons
        #of points in polygon
        x y z
        x y z
        ...
        #of edges
        Pnt1 Pnt2
        Pnt1 Pnt2
        ...

    Explanation: Pnt1 and Pnt2 correspond to which vertices there is an edge between
        So if there is an edge from Pnt1 to Pnt2, (x1, y1, z1) and (x2, y2, z2) have an edge between them
        */