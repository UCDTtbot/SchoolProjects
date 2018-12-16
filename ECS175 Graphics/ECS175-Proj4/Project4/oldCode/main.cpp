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
#include "P3_Funcs.h"
#include <map>
#include <algorithm>

using namespace std;

//Variables
float *PIXEL_BUFFER_MAIN; // [width * height * RGB] | [x * 3 + y * width * 3] for 0, then add 1 and then 2 
float *PIXEL_BUFFER_XY;
float *PIXEL_BUFFER_XZ;
float *PIXEL_BUFFER_YZ;
float *XY_Rast, *XZ_Rast, *YZ_Rast;
static int CURRENT_POLY = 0;    //Poly ID
static int MainWindow;  //Global for holding the mainWindow ID
static int subWindow_1, subWindow_2, subWindow_3;
static int WIDTH = 900, HEIGHT = 900;
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

                    //Lighting and Shading stuff
triple<float>* Intens_XY;
triple<float>* Intens_XZ;
triple<float>* Intens_YZ;
//static map<triple<int>, triple<float> > intensity;


//NDC
static vector<pair<TPnt, TPnt> > NDC_XY; //Vector of 3D Pnts, one for each Polygon
static vector<pair<TPnt, TPnt> > NDC_XZ;
static vector<pair<TPnt, TPnt> > NDC_YZ;


//static Edge rotateAxis;
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

                    //Degrees
const float  PI_F = 3.14159265358979f; //PI for degrees to radians
const float D_TO_R = PI_F / 180;    //Use to convert degrees_to_radian
                                    
static int totalPolys = 0;  //Total polys made
static int maxPolys = 3;    //Max number of polys allowed to be made in this program
vector<THPolygon> Poly;   //Vector for holding polys that have been 

//void phong(THPolygon curPoly, float I_amb, float I_source, int nPhong, TPnt x (light source), TPnt f (viewing vector), triple k_amb, triple k_dif, triple k_spec)
//phong(curPoly, 0.3, 1, 10, TPnt(3, 5, 6), TPnt(-3, 2, 0), triple<float>(0.9, 0.4, 0.6), triple<float>(0.5, 0.6, 0.8), triple<float>(0.4, 0.3, 0.6));
float I_amb = 0.3;
float I_source = 1;
int nPhong = 10;
TPnt lightPos(3, 5, 6);
TPnt viewer(-3, 2, 0);
triple<float> k_amb(0.9, 0.4, 0.6);
triple<float> k_diff(0.5, 0.6, 0.8);
triple<float> k_spec(0.4, 0.3, 0.6);

//Functions

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
void lineDDA(int buffer, float X0, float Y0, float X, float Y, THPolygon, int, float R, float G, float B);  //Draws line from initial point to final point. Also takes RGB color
void lineBres(int buffer, int X0, int Y0, int X_End, int Y_End, THPolygon, int, float R, float G, float B); //See above
void consoleMenu(); //Function for setting up the console menu
void drawLines(int pStep, vector<Face> sortedFaces);   //Function that draws lines to pixel buffer using either lineDDA or lineBres
            
            //Project 3 Stuff
void drawFaces();   //Need this to do painters
void setEdgeIntens(int buffer, int x, int y, THPolygon, int);
void fillDDA(int buffer, float X0, int Y0, int X_End, int Y_End, THPolygon curPoly, triple<float> I1, triple<float> I2, float R, float G, float B);
void scanIntens(int buffer, int x, int y, THPolygon curPoly, triple<float> I1, triple<float> I2, float X0, float X_End);
void newPhong();
void phong(THPolygon &curPoly, float I_amb, float I_source, int nPhong, TPnt x, TPnt f, triple<float> k_amb, triple<float> k_dif, triple<float> k_spec);



                    //Transforms
void translate(float x, float y, float z);   //Translation func
void rotate(float angle);       //Rotation func
void scale(float alpha, float beta, float zeta);    //Scaling func
void redisplayAllWindows();

                    //Normalization
void getMasters();
void normalize(int, int, Face);
void normalizeRotation(TPnt, TPnt);

                    //File IO
void getFileName();
void loadFile();    //Function for loading from file
void saveFile();    //Function for saving to file

                    //Utility
void animate(int value);
void displayVerts(int PolyID);
bool mySort(Face one, Face two);

                    //Rasterize
void raster();
void rasterLines(int buffer, float X0, float Y0, float X_End, float Y_End, int R, int G, int B);
void rastPixel(int buffer, float x, float y, int R, int G, int B);
void XY_Raster(THPolygon curPoly);
void XZ_Raster(THPolygon curPoly);
void YZ_Raster(THPolygon curPoly);

                    //Halftone
void halftone();

//      Main
int main(int argc, char *argv[])
{
    char fileLoad; //Variable to load from file or not
    cout << "Welcome.\n";
    //Choose how to draw lines
    //cout << "Please choose a line drawing algorithm: \n";
    //cout << "1. DDA\n2. Bres\n"; cin >> drawAlg;
    //while (drawAlg < 1 || drawAlg > 2)
    //{
    //    cout << "Invalid input...\n";
    //    cout << "Please choose a line drawing algorithm: \n";
    //    cout << "1. DDA\n2. Bres\n"; cin >> drawAlg;
    //}
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
    displayVerts(CURRENT_POLY);
    glutMainLoop();

    delete[] PIXEL_BUFFER_XY;
    delete[] PIXEL_BUFFER_XZ;
    delete[] PIXEL_BUFFER_YZ;
    delete[] Intens_XY;
    delete[] Intens_XZ;
    delete[] Intens_YZ;
    return 0;
}

//      Functions for setting up OpenGL
void init()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

    PIXEL_BUFFER_MAIN = new float[WIDTH * HEIGHT * 3];
    PIXEL_BUFFER_XY = new float[subWidth_1 * subHeight_1 * 3];
    PIXEL_BUFFER_XZ = new float[subWidth_2 * subHeight_2 * 3];
    PIXEL_BUFFER_YZ = new float[subWidth_3 * subHeight_3 * 3];
    Intens_XY = new triple<float>[subWidth_1 * subHeight_1 * 3];
    Intens_XZ = new triple<float>[subWidth_2 * subHeight_2 * 3];
    Intens_YZ = new triple<float>[subWidth_3 * subHeight_3 * 3];

    for (int step = 0; step < WIDTH * HEIGHT * 3; step++)//Set initial buffer to a blank white backround
        PIXEL_BUFFER_MAIN[step] = 0.0f;
    for (int step = 0; step < subWidth_1 * subHeight_1 * 3; step++)             //xy
        PIXEL_BUFFER_XY[step] = 0.6f;
    for (int step = 0; step < subWidth_2 * subHeight_2 * 3; step++)             //xz
        PIXEL_BUFFER_XZ[step] = 0.4f;
    for (int step = 0; step < subWidth_3 * subHeight_3 * 3; step++)             //yz
        PIXEL_BUFFER_YZ[step] = 0.2f;
    
    for (int step = 0; step < totalPolys; step++)
    {
        pair<TPnt, TPnt> *newPnt_XY = new pair<TPnt, TPnt>; newPnt_XY->first.z = -1; newPnt_XY->second.z = -1;
        pair<TPnt, TPnt> *newPnt_XZ = new pair<TPnt, TPnt>; newPnt_XZ->first.y = -1; newPnt_XZ->second.y = -1;
        pair<TPnt, TPnt> *newPnt_YZ = new pair<TPnt, TPnt>; newPnt_YZ->first.x = -1; newPnt_YZ->second.x = -1;

        NDC_XY.push_back(*newPnt_XY);
        NDC_XZ.push_back(*newPnt_XZ);
        NDC_YZ.push_back(*newPnt_YZ);
    }

    drawFaces();
    raster();

    glutSetWindow(MainWindow);

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

//      Line Drawing Functions
void setPixel(int Buffer, float x, float y, float R, float G, float B)
{   //Backround is Init to 1.0f = white, set pixel RGB to 0.0f for black

    //Main = 0 | XY = 1 | XZ = 2 | YZ = 3
    if (Buffer == mainBuf) //Main
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                PIXEL_BUFFER_MAIN[(((int)x) * 3 + ((int)y) * WIDTH * 3) + step] = R;  //Set R
            else if (step == 1)
                PIXEL_BUFFER_MAIN[(((int)x) * 3 + ((int)y) * WIDTH * 3) + step] = G;  //Set G
            else if (step == 2)
                PIXEL_BUFFER_MAIN[(((int)x) * 3 + ((int)y) * WIDTH * 3) + step] = B;  //Set B
        }
    }
    else if (Buffer == xyBuf) //XY
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                PIXEL_BUFFER_XY[(((int)x) * 3 + ((int)y) * subWidth_1 * 3) + step] = R;  //Set R
            else if (step == 1)
                PIXEL_BUFFER_XY[(((int)x) * 3 + ((int)y) * subWidth_1 * 3) + step] = G;  //Set G
            else if (step == 2)
                PIXEL_BUFFER_XY[(((int)x) * 3 + ((int)y) * subWidth_1 * 3) + step] = B;  //Set B
        }
    }
    else if (Buffer == xzBuf) //XZ
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                PIXEL_BUFFER_XZ[(((int)x) * 3 + ((int)y) * subWidth_2 * 3) + step] = R;  //Set R
            else if (step == 1)
                PIXEL_BUFFER_XZ[(((int)x) * 3 + ((int)y) * subWidth_2 * 3) + step] = G;  //Set G
            else if (step == 2)
                PIXEL_BUFFER_XZ[(((int)x) * 3 + ((int)y) * subWidth_2 * 3) + step] = B;  //Set B
        }
    }
    else if (Buffer == yzBuf) //YZ
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                PIXEL_BUFFER_YZ[(((int)x) * 3 + ((int)y) * subWidth_3 * 3) + step] = R;  //Set R
            else if (step == 1)
                PIXEL_BUFFER_YZ[(((int)x) * 3 + ((int)y) * subWidth_3 * 3) + step] = G;  //Set G
            else if (step == 2)
                PIXEL_BUFFER_YZ[(((int)x) * 3 + ((int)y) * subWidth_3 * 3) + step] = B;  //Set B
        }
    }
    else
        ;
}

void lineDDA(int buffer, float X0, float Y0, float X_End, float Y_End, THPolygon curPoly, int curEdge, float R, float G, float B)
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
        setEdgeIntens(buffer, round(Xn), round(Yn), curPoly, curEdge);
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc;
            setEdgeIntens(buffer, round(Xn), round(Yn), curPoly, curEdge);
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

        setEdgeIntens(buffer, round(Xn), round(Yn), curPoly, curEdge);
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc; //Increment X and Y by their respective values
            Yn += yInc;
            setEdgeIntens(buffer, round(Xn), round(Yn), curPoly, curEdge);
        }
    }
}

void lineBres(int buffer, int X0, int Y0, int X_End, int Y_End, THPolygon curPoly, int curEdge, float R, float G, float B)
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

void drawLines(int pStep, vector<Face> sortedFaces)
{

    THPolygon curPoly = Poly[pStep];
        //void phong(THPolygon curPoly, float I_amb, float I_source, int nPhong, TPnt x (light source), TPnt f (viewing vector), triple k_amb, triple k_dif, triple k_spec)
        //default datas, TODO: ADD ACTUAL PARAMS
    phong(curPoly, I_amb, I_source, nPhong, lightPos, viewer, k_amb, k_diff, k_spec);

    if (drawAlg == 2) // do Bres
    {
        for (int eStep = 0; eStep < curPoly.totalEdges; eStep++)
        {
            for (vector<Face>::iterator it = sortedFaces.end(); it != sortedFaces.begin(); --it)
            {
                for (int eStep = 0; eStep < 3; eStep++)
                {
                    normalize(pStep, eStep, *it);
                    lineBres(xyBuf, NDC_XY[pStep].first.x, NDC_XY[pStep].first.y, NDC_XY[pStep].second.x, NDC_XY[pStep].second.y, curPoly, eStep, 0.0f, 0.0f, 0.0f);
                    lineBres(xzBuf, NDC_XZ[pStep].first.x, NDC_XZ[pStep].first.z, NDC_XZ[pStep].second.x, NDC_XZ[pStep].second.z, curPoly, eStep, 0.0f, 0.0f, 0.0f);
                    lineBres(yzBuf, NDC_YZ[pStep].first.y, NDC_YZ[pStep].first.z, NDC_YZ[pStep].second.y, NDC_YZ[pStep].second.z, curPoly, eStep, 0.0f, 0.0f, 0.0f);
                }
            }
        }
    }
    else //Default to DDA
    {
        for (vector<Face>::iterator it = sortedFaces.end()-1; it != sortedFaces.begin(); --it)
       {
           for (int eStep = 0; eStep < 3; eStep++)
           {
               normalize(pStep, eStep, *it);
               lineDDA(xyBuf, NDC_XY[pStep].first.x, NDC_XY[pStep].first.y, NDC_XY[pStep].second.x, NDC_XY[pStep].second.y, curPoly, eStep, 0.0f, 0.0f, 0.0f);
               lineDDA(xzBuf, NDC_XZ[pStep].first.x, NDC_XZ[pStep].first.z, NDC_XZ[pStep].second.x, NDC_XZ[pStep].second.z, curPoly, eStep, 0.0f, 0.0f, 0.0f);
               lineDDA(yzBuf, NDC_YZ[pStep].first.y, NDC_YZ[pStep].first.z, NDC_YZ[pStep].second.y, NDC_YZ[pStep].second.z, curPoly, eStep, 0.0f, 0.0f, 0.0f);
           }
       }
    }
    
    //P3 Calls
    //P3 Pipeline goes here
}

bool mySort(Face one, Face two) 
{ 
    float deep1, deep2;
    deep1 = one.Pnt1.z;
    if (one.Pnt2.z > deep1)
        deep1 = one.Pnt2.z;
    if (one.Pnt3.z > deep1)
        deep1 = one.Pnt3.z;
    deep2 = two.Pnt1.z;
    if (two.Pnt2.z > deep2)
        deep2 = two.Pnt2.z;
    if (two.Pnt3.z > deep2)
        deep2 = two.Pnt3.z;

    return (deep1 < deep2);
}

void drawFaces()
{
    //Reset Buffers
    for (int step = 0; step < WIDTH * HEIGHT * 3; step++)//Set initial buffer to a blank white backround
        PIXEL_BUFFER_MAIN[step] = 0.0f;
    for (int step = 0; step < subWidth_1 * subHeight_1 * 3; step++)             //xy
        PIXEL_BUFFER_XY[step] = 0.6f;
    for (int step = 0; step < subWidth_2 * subHeight_2 * 3; step++)             //xz
        PIXEL_BUFFER_XZ[step] = 0.4f;
    for (int step = 0; step < subWidth_3 * subHeight_3 * 3; step++)             //yz
        PIXEL_BUFFER_YZ[step] = 0.2f;

    for (int pStep = 0; pStep < totalPolys; pStep++)   //For Each Poly and For each edge IN that poly
    {
        //Work with 1 polygon at a time. Sort the faces by z depth
        //sort faces by z depth
        vector<Face> sortedFaces;
        THPolygon curPoly = Poly[pStep];
        for (int i = 0; i < curPoly.totalFaces; i++)
            sortedFaces.push_back(curPoly.faces[i]);
        sort(sortedFaces.begin(), sortedFaces.end(), mySort);
        cout << "Sorted faces: \n";
        for (int i = 0; i < curPoly.totalFaces; i++)
            cout << i << ": " << sortedFaces[i].Pnt1.z << " | " << sortedFaces[i].Pnt2.z << " | " << sortedFaces[i].Pnt3.z << "\n";
        //There should be extra steps but I'm running out of time, just going to draw as it is currently sorted
        
        drawLines(pStep, sortedFaces);
        
    }
}

void setEdgeIntens(int buffer, int x, int y, THPolygon curPoly, int curEdge)
{
    //(((int)x) * 3 + ((int)y) * WIDTH * 3) + step
    //If edge NOT horizontal
    //I(point) = ((y(point) - y1) / (y2 - y1)) * I2 + ((y2 - y(point)) / (y2 - y1)) * I1
    //If edge horizontal
    //I(point) = ((x(point) - x1) / (x2 - x1)) * I2 + ((x2 - x(point)) / (x2 - x1)) * I1
    //Always: y1 < y(point) < y2
    //Always: x1 < x(point) < x2
    int Y1 = curPoly.edges[curEdge].first.y;
    int Y2 = curPoly.edges[curEdge].second.y;
    if (Y1 > Y2)
    {
        int yTemp = Y2;
        Y2 = Y1;
        Y1 = yTemp;
    }
    int X1 = curPoly.edges[curEdge].first.x;
    int X2 = curPoly.edges[curEdge].second.x;
    if (X1 > X2)
    {
        int xTemp = X2;
        X2 = X1;
        X1 = xTemp;
    }
    TPnt I1 = curPoly.vertices[curPoly.edges[curEdge].fstCoord - 1];
    TPnt I2 = curPoly.vertices[curPoly.edges[curEdge].scdCoord - 1];

    if (buffer == xyBuf)
    {
        if (curPoly.edges[curEdge].slope == 0)  //Use x's
        {
            R = ((x - X1) / (float)(X2 - X1)) * I2.R + ((X2 - x) / (float)(X2 - X1)) * I1.R;
            B = ((x - X1) / (float)(X2 - X1)) * I2.G + ((X2 - x) / (float)(X2 - X1)) * I1.G;
            G = ((x - X1) / (float)(X2 - X1)) * I2.B + ((X2 - x) / (float)(X2 - X1)) * I1.B;
            setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
        }
        else    //Use y's
        {
            R = ((x - Y1) / (float)(Y2 - Y1)) * I2.R + ((Y2 - x) / (float)(Y2 - Y1)) * I1.R;
            B = ((x - Y1) / (float)(Y2 - Y1)) * I2.G + ((Y2 - x) / (float)(Y2 - Y1)) * I1.G;
            G = ((x - Y1) / (float)(Y2 - Y1)) * I2.B + ((Y2 - x) / (float)(Y2 - Y1)) * I1.B;
            setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
        }
    }
    else if (buffer == xzBuf)
    {
        if (curPoly.edges[curEdge].slope == 0)  //Use x's
        {
            R = ((x - X1) / (float)(X2 - X1)) * I2.R + ((X2 - x) / (float)(X2 - X1)) * I1.R;
            B = ((x - X1) / (float)(X2 - X1)) * I2.G + ((X2 - x) / (float)(X2 - X1)) * I1.G;
            G = ((x - X1) / (float)(X2 - X1)) * I2.B + ((X2 - x) / (float)(X2 - X1)) * I1.B;
            setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
        }
        else    //Use y's
        {
            R = ((x - Y1) / (float)(Y2 - Y1)) * I2.R + ((Y2 - x) / (float)(Y2 - Y1)) * I1.R;
            B = ((x - Y1) / (float)(Y2 - Y1)) * I2.G + ((Y2 - x) / (float)(Y2 - Y1)) * I1.G;
            G = ((x - Y1) / (float)(Y2 - Y1)) * I2.B + ((Y2 - x) / (float)(Y2 - Y1)) * I1.B;
            setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
        }
    }
    else if (buffer == yzBuf)
    {
        if (curPoly.edges[curEdge].slope == 0)  //Use x's
        {
            R = ((x - X1) / (float)(X2 - X1)) * I2.R + ((X2 - x) / (float)(X2 - X1)) * I1.R;
            B = ((x - X1) / (float)(X2 - X1)) * I2.G + ((X2 - x) / (float)(X2 - X1)) * I1.G;
            G = ((x - X1) / (float)(X2 - X1)) * I2.B + ((X2 - x) / (float)(X2 - X1)) * I1.B;
            setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
        }
        else    //Use y's
        {
            R = ((x - Y1) / (float)(Y2 - Y1)) * I2.R + ((Y2 - x) / (float)(Y2 - Y1)) * I1.R;
            B = ((x - Y1) / (float)(Y2 - Y1)) * I2.G + ((Y2 - x) / (float)(Y2 - Y1)) * I1.G;
            G = ((x - Y1) / (float)(Y2 - Y1)) * I2.B + ((Y2 - x) / (float)(Y2 - Y1)) * I1.B;
            setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
        }
    }
}

//      Normalization
void normalize(int pStep, int eStep, Face curFace)
{
    getMasters();
    float xy_1X, xy_1Y, xy_2X, xy_2Y;
    float xz_1X, xz_1Z, xz_2X, xz_2Z;
    float yz_1Y, yz_1Z, yz_2Y, yz_2Z;
    float masterMax = MastMax, masterMin = MastMin;
    //(point - masterMin) / (masterMax - masterMin)
    xy_1X = ((curFace.edges[eStep].first.x - masterMin) / (float)(masterMax - masterMin));
    xy_1Y = ((curFace.edges[eStep].first.y - masterMin) / (float)(masterMax - masterMin));
    xy_2X = ((curFace.edges[eStep].second.x - masterMin) / (float)(masterMax - masterMin));
    xy_2Y = ((curFace.edges[eStep].second.y - masterMin) / (float)(masterMax - masterMin));

    xz_1X = ((curFace.edges[eStep].first.x - masterMin) / (float)(masterMax - masterMin));
    xz_1Z = ((curFace.edges[eStep].first.z - masterMin) / (float)(masterMax - masterMin));
    xz_2X = ((curFace.edges[eStep].second.x - masterMin) / (float)(masterMax - masterMin));
    xz_2Z = ((curFace.edges[eStep].second.z - masterMin) / (float)(masterMax - masterMin));

    yz_1Y = ((curFace.edges[eStep].first.y - masterMin) / (float)(masterMax - masterMin));
    yz_1Z = ((curFace.edges[eStep].first.z - masterMin) / (float)(masterMax - masterMin));
    yz_2Y = ((curFace.edges[eStep].second.y - masterMin) / (float)(masterMax - masterMin));
    yz_2Z = ((curFace.edges[eStep].second.z - masterMin) / (float)(masterMax - masterMin));

    //cout << "Printing out Normalized Coords. MasterMin: " << masterMin << " MasterMax: " << masterMax << "\n";
    //cout << "XY, edge #" << eStep << ":\n";
    //cout << "First: (" << xy_1X << ", " << xy_1Y << ") Second: (" << xy_2X << ", " << xy_2Y << ") || " << "First: (" << xy_1X * subWidth_1 << ", " << xy_1Y * (subHeight_1 - 1) << ") Second : (" << xy_2X * subWidth_1 << ", " << xy_2Y * (subHeight_1 - 1) << ") \n";

    //cout << "XZ, edge #" << eStep << ":\n";
    //cout << "First: (" << xz_1X << ", " << xz_1Z << ") Second: (" << xz_2X << ", " << xz_2Z << ")\n";

    //cout << "YZ, edge #" << eStep << ":\n";
    //cout << "First: (" << yz_1Y << ", " << yz_1Z << ") Second: (" << yz_2Y << ", " << yz_2Z << ")\n";

    NDC_XY[pStep].first.x = xy_1X * (subWidth_1 / 2); NDC_XY[pStep].first.y = xy_1Y * ((subHeight_1 - 1) / 2);
    NDC_XY[pStep].second.x = xy_2X * (subWidth_1 / 2); NDC_XY[pStep].second.y = xy_2Y * ((subHeight_1 - 1) / 2);

    NDC_XZ[pStep].first.x = xz_1X * (subWidth_2 / 2); NDC_XZ[pStep].first.z = xz_1Z * ((subHeight_2 - 1) / 2);
    NDC_XZ[pStep].second.x = xz_2X * (subWidth_2 / 2); NDC_XZ[pStep].second.z = xz_2Z * ((subHeight_2 - 1) / 2);

    NDC_YZ[pStep].first.y = yz_1Y * (subWidth_3 / 2); NDC_YZ[pStep].first.z = yz_1Z * ((subHeight_3 - 1) / 2);
    NDC_YZ[pStep].second.y = yz_2Y * (subWidth_3 / 2); NDC_YZ[pStep].second.z = yz_2Z * ((subHeight_3 - 1) / 2);
}

//void normalizeRotation(TPnt Pnt1, TPnt Pnt2)
//{
//    getMasters();
//    float xy_1X, xy_1Y, xy_2X, xy_2Y;
//    float xz_1X, xz_1Z, xz_2X, xz_2Z;
//    float yz_1Y, yz_1Z, yz_2Y, yz_2Z;
//    float masterMax = MastMax, masterMin = MastMin;
//    //(point - masterMin) / (masterMax - masterMin)
//    xy_1X = (((Pnt1.x - masterMin) / (float)(masterMax - masterMin)) * (subWidth_1 / 2));
//    xy_1Y = (((Pnt1.y - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_1 - 1) / 2));
//    xy_2X = (((Pnt2.x - masterMin) / (float)(masterMax - masterMin)) * (subWidth_1 / 2));
//    xy_2Y = (((Pnt2.y - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_1 - 1) / 2));
//
//    xz_1X = (((Pnt1.x - masterMin) / (float)(masterMax - masterMin)) * (subWidth_2 / 2));
//    xz_1Z = (((Pnt1.z - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_2 - 1) / 2));
//    xz_2X = (((Pnt2.x - masterMin) / (float)(masterMax - masterMin)) * (subWidth_2 / 2));
//    xz_2Z = (((Pnt2.z - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_2 - 1) / 2));
//
//    yz_1Y = (((Pnt1.y - masterMin) / (float)(masterMax - masterMin)) * (subWidth_3 / 2));
//    yz_1Z = (((Pnt1.z - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_3 - 1) / 2));
//    yz_2Y = (((Pnt2.y - masterMin) / (float)(masterMax - masterMin)) * (subWidth_3 / 2));
//    yz_2Z = (((Pnt2.z - masterMin) / (float)(masterMax - masterMin)) * ((subHeight_3 - 1) / 2));
//
//    if (rotating == false)
//    {
//        lineDDA(xyBuf, xy_1X, xy_1Y, xy_2X, xy_2Y, 0.8f, 0.8f, 0.8f);
//        lineDDA(xzBuf, xz_1X, xz_1Z, xz_2X, xz_2Z, 0.6f, 0.6f, 0.6f);
//        lineDDA(yzBuf, yz_1Y, yz_1Z, yz_2Y, yz_2Z, 0.4f, 0.4f, 0.4f);
//    }
//    else
//    {
//        lineDDA(xyBuf, xy_1X, xy_1Y, xy_2X, xy_2Y, 0.0f, 0.0f, 0.0f);
//        lineDDA(xzBuf, xz_1X, xz_1Z, xz_2X, xz_2Z, 0.0f, 0.0f, 0.0f);
//        lineDDA(yzBuf, yz_1Y, yz_1Z, yz_2Y, yz_2Z, 0.0f, 0.0f, 0.0f);
//    }
//}

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

//      Menu's
void createMenu()
{
    glutSetWindow(MainWindow);
    subMenuID1 = glutCreateMenu(menu);  //SubMenu for Polygon IDs
    glutAddMenuEntry("Polygon #0", 1);
    glutAddMenuEntry("Polygon #1", 2);
    glutAddMenuEntry("Polygon #2", 3);
    menuID = glutCreateMenu(menu);  //Setup main Menu
    glutAddSubMenu("Select Polygon", subMenuID1);
    glutAddMenuEntry("Display Vertices and Edges", 9);
    glutAddMenuEntry("Display Phong Info", 10);
    glutAddMenuEntry("Load Polygons", 8);
    glutAddMenuEntry("Phong Input", 11);
    glutAddMenuEntry("Half-Tone", 12);
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
        phong(Poly[CURRENT_POLY], I_amb, I_source, nPhong, lightPos, viewer, k_amb, k_diff, k_spec);
    }
    else if (num == 11)
    {
        newPhong();
    }
    else if (num == 12)
    {
        halftone();
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

//      Transforms
//void translate(float xIn, float yIn, float zIn)
//{
//    //Move every point of the polygon by X and Y
//    //Does not support going under (0,0)
//    THPolygon curPoly = Poly[CURRENT_POLY];
//    float newX = 0, newY = 0, newZ = 0;   //New x, y, z
//    for (int step = 0; step < curPoly.totalEdges; step++)
//    {
//        newX = curPoly.edges[step].first->x + xIn;   curPoly.edges[step].first->x = newX;
//        newY = curPoly.edges[step].first->y + yIn;   curPoly.edges[step].first->y = newY;
//        newZ = curPoly.edges[step].first->z + zIn;   curPoly.edges[step].first->z = newZ;
////        cout << "First, X: " << newX << " Y: " << newY << " Z: " << newZ << endl;
//        newX = curPoly.edges[step].second->x + xIn;  curPoly.edges[step].second->x = newX;
//        newY = curPoly.edges[step].second->y + yIn;  curPoly.edges[step].second->y = newY;
//        newZ = curPoly.edges[step].second->z + zIn;  curPoly.edges[step].second->z = newZ;
////        cout << "Second, X: " << newX << " Y: " << newY << " Z: " << newZ << endl;
//    }
//    for (int vStep = 0; vStep < curPoly.totalVerts; vStep++)
//    {
//        newX = curPoly.vertices[vStep].x + xIn;
//        newY = curPoly.vertices[vStep].y + yIn;
//        newZ = curPoly.vertices[vStep].z + zIn;
//        curPoly.setVert(vStep, newX, newY, newZ);
//    }
//    Poly[CURRENT_POLY] = curPoly;
//    drawLines();
//}
//
//void rotate(float angle)
//{
//    /*
//    Rotation is very similar to scaling below. Will need to be done by centroid
//    The (homogeneous) rotation matrix is:   (cos  -sin | 0)  (2)
//    (sin   cos | 0)  (1)
//    (-------------)  (-)
//    (0     0   | 1)  (1)
//    Steps for rotation:
//    1. Translation (by -(centroid) )
//    2. Rotate each vertice by the rotation matrix
//    3. Translation (by (centroid) )
//    */
//    THPolygon curPoly = Poly[CURRENT_POLY];
//    TPnt Pnt1 = *rotateAxis.first;
//    TPnt Pnt2 = *rotateAxis.second;
//    float dx = Pnt2.x - Pnt1.x;
//    float dy = Pnt2.y - Pnt1.y;
//    float dz = Pnt2.z - Pnt1.z;
//    float I = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
//    float ux = dx / (float)I;
//    float uy = dy / (float)I;
//    float uz = dz / (float)I;
//    float newX = 0, newY = 0, newZ = 0;
//    float x, y, z;
//    float cosA = cos(angle*D_TO_R), sinA = sin(angle*D_TO_R);
//    float oneC = (1 - cosA);
//    float mat1, mat2, mat3, mat4, mat5, mat6, mat7, mat8, mat9;
//    mat1 = ux*ux*oneC + cosA;           //These correspond to the matrix found in the book, where it is a 3x3 matrix. mat1 is top left, mat2 is top middle, mat9 is bottom right
//    mat2 = ux*uy*oneC - (uz*sinA);
//    mat3 = ux*uz*oneC + (uy*sinA);
//    mat4 = uy*ux*oneC + (uz*sinA);
//    mat5 = uy*uy*oneC + cosA;
//    mat6 = uy*uz*oneC - (ux*sinA);
//    mat7 = uz*ux*oneC - (uy*sinA);
//    mat8 = uz*uy*oneC + (ux*sinA);
//    mat9 = uz*uz*oneC + cosA;
//    //First translate by -centroid
//    translate(-(curPoly.centroid.x), -(curPoly.centroid.y), -(Poly[CURRENT_POLY].centroid.z));
//    //Rotate - Edges
//    for (int step = 0; step < curPoly.totalEdges; step++)
//    {
//        //Edge Pnt1
//        x = curPoly.edges[step].first->x;
//        y = curPoly.edges[step].first->y;
//        z = curPoly.edges[step].first->z;
//        newX = x*mat1 + y*mat2 + z*mat3;
//        newY = x*mat4 + y*mat5 + z*mat6;
//        newZ = x*mat7 + y*mat8 + z*mat9;
//        curPoly.edges[step].first->x = newX;
//        curPoly.edges[step].first->y = newY;
//        curPoly.edges[step].first->z = newZ;
//
//        //Edge Pnt2
//        x = curPoly.edges[step].second->x;
//        y = curPoly.edges[step].second->y;
//        z = curPoly.edges[step].second->z;
//        newX = x*mat1 + y*mat2 + z*mat3;
//        newY = x*mat4 + y*mat5 + z*mat6;
//        newZ = x*mat7 + y*mat8 + z*mat9;
//        curPoly.edges[step].second->x = newX;
//        curPoly.edges[step].second->y = newY;
//        curPoly.edges[step].second->z = newZ;
//
//    }
//    //Rotate - Verts
//    for (int step = 0; step < curPoly.totalVerts; step++)
//    {
//        x = curPoly.vertices[step].x;
//        y = curPoly.vertices[step].y;
//        z = curPoly.vertices[step].z;
//        newX = x*mat1 + y*mat2 + z*mat3;
//        newY = x*mat4 + y*mat5 + z*mat6;
//        newZ = x*mat7 + y*mat8 + z*mat9;
//        curPoly.setVert(step, newX, newY, newZ);
//    }
//    //Translate back by centroid
//    translate(curPoly.centroid.x, curPoly.centroid.y, curPoly.centroid.z);
//    Poly[CURRENT_POLY] = curPoly;
//    drawLines();
//}
//
//void scale(float alpha, float beta, float zeta)
//{
//    /*
//    scale w.r.t center
//    Using a homogeneous scaling matrix of   (alpha 0    | 0)
//    (0     beta | 0)
//    (--------------)
//    (0     0    | 1)
//    Apply the matrix to each point (Scaling by origin)
//    For example, a polygon with vertices (0, 0) (0, 1) (1, 1) (1, 0) when applied to the scaling factor would then be:
//    (0, 0) -> (0, 0)
//    (1, 0) -> (2, 0)
//    (1, 1) -> (2, 2)
//    (1, 0) -> (2, 0)
//    What we see above is not exactly what we want.
//    We must move the center (centroid) of the polygon to the origin | centroid is calculated by taking the avg of all vertices
//    ---- Three operations to scale by centroid:
//    1. Translation (By -(centroid) )
//    2. Scale (By scale matrix in every direction) - after translation, scaling factor will be applied to every point
//    3. Translation (By (centroid) )
//    */
//    THPolygon curPoly = Poly[CURRENT_POLY];
//    //First translate by -centroid
//    translate(-(curPoly.centroid.x), -(curPoly.centroid.y), -(curPoly.centroid.z));
//    //Scale - edges
//    for (int step = 0; step < curPoly.totalEdges; step++)
//    {
//        float newX1 = 0, newY1 = 0, newZ1 = 0;
//        float newX2 = 0, newY2 = 0, newZ2 = 0;
//        newX1 += curPoly.edges[step].first->x; newX1 *= alpha;    curPoly.edges[step].first->x = newX1;   //New vert = X * alpha
//        newY1 += curPoly.edges[step].first->y; newY1 *= beta;     curPoly.edges[step].first->y = newY1;   //New vert = Y * beta
//        newZ1 += curPoly.edges[step].first->z; newZ1 *= zeta;     curPoly.edges[step].first->z = newZ1;   //New vert = Z * zeta
//
//        newX2 += curPoly.edges[step].second->x; newX2 *= alpha;   curPoly.edges[step].second->x = newX2;
//        newY2 += curPoly.edges[step].second->y; newY2 *= beta;    curPoly.edges[step].second->y = newY2;
//        newZ2 += curPoly.edges[step].second->z; newZ2 *= zeta;    curPoly.edges[step].second->z = newZ2;
//    }
//    //Verts
//    for (int vStep = 0; vStep < curPoly.totalVerts; vStep++)
//    {
//        float newX, newY, newZ;
//        newX = curPoly.vertices[vStep].x * alpha;
//        newY = curPoly.vertices[vStep].y * beta;
//        newZ = curPoly.vertices[vStep].z * zeta;
//        curPoly.setVert(vStep, newX, newY, newZ);
//    }
//    //Translate back by centroid
//    translate(curPoly.centroid.x, curPoly.centroid.y, curPoly.centroid.z);
//    Poly[CURRENT_POLY] = curPoly;
//    drawLines();
//}

//      File IO
void loadFile()
{
    //Files will have very strict formatting. Will crash unless its very specifically formatted
    ifstream inFile;
    inFile.open(fileName.c_str());
    string line;
    int totalVerts = 0, totalEdges = 0, totalFaces = 0;
    int pnt1, pnt2, pnt3;
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
            for (int normStep = 0; normStep < totalVerts; normStep++)
            {
                inFile >> newPoly.vertices[normStep].vertexNormal.i;
                inFile >> newPoly.vertices[normStep].vertexNormal.j;
                inFile >> newPoly.vertices[normStep].vertexNormal.k;
            }
            inFile >> totalEdges;
            for (int edgeStep = 0; edgeStep < totalEdges; edgeStep++)
            {
                inFile >> pnt1; inFile >> pnt2;
                newPoly.createEdge(newPoly.vertices[pnt1 - 1], newPoly.vertices[pnt2 - 1], pnt1, pnt2);
            }
            inFile >> totalFaces;
            for (int faceStep = 0; faceStep < totalFaces; faceStep++)
            {
                inFile >> pnt1; inFile >> pnt2; inFile >> pnt3;
                newPoly.createFace(newPoly.vertices[pnt1 - 1], newPoly.vertices[pnt2 - 1], newPoly.vertices[pnt3 - 1], pnt1, pnt2, pnt3);
            }
            for (int normStep = 0; normStep < totalFaces; normStep++)
            {
                inFile >> newPoly.faces[normStep].normal.i;
                inFile >> newPoly.faces[normStep].normal.j;
                inFile >> newPoly.faces[normStep].normal.k;
            }
            Poly.push_back(newPoly);
           // Poly[step].getCentroid();
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

//      Utility
//void animate(int value)
//{
//    if (value != 0)
//    {
//        rotateAxis.first->x = 0; rotateAxis.first->y = 0; rotateAxis.first->z = 0;
//        rotateAxis.second->x = 500; rotateAxis.second->y = 500; rotateAxis.second->z = 500;
//        rotate(15);
//        glutTimerFunc(250, animate, 1);
//        redisplayAllWindows();
//    }
//    else
//    {
//        glutTimerFunc(0, animate, 0);
//        redisplayAllWindows();
//    }
//}

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
    cout << "Defined faces for polygon #" << PolyID << " are:\n";
    for (int faceStep = 0; faceStep < curPoly.totalFaces; faceStep++)
    {
        cout << curPoly.faces[faceStep].first << " " << curPoly.faces[faceStep].second << " " << curPoly.faces[faceStep].third << " \n";
    }

}

//      Rasterization
void raster()
{
    XY_Rast = new float[subWidth_1 * subHeight_1 * 3];
    XZ_Rast = new float[subWidth_2 * subHeight_2 * 3];
    YZ_Rast = new float[subWidth_3 * subHeight_3 * 3];
    int workingPoly = 0;
    float xy_x1, xy_y1, xy_x2, xy_y2;
    float xz_x1, xz_z1, xz_x2, xz_z2;
    float yz_y1, yz_z1, yz_y2, yz_z2;
    /*
    Scan from y = 0 to y = MAX
    Find all X intercepts for each edge ( see book )
    From list of X intercepts, fill pairwise
    List should be someting like    Y0 = (x1, x2, x3, x4)
    Y1 = (x1, x2)
    Would then fill from x1 - x2, x3 - x4 for Y0 then x1 - x2 for Y1
    */


    /**********************************************************************\
        For each Poly
            For each face
                draw edges using interpolation in book
                    scan line between edges and interpolate RGB

        No more -1 checking, only non-1 checking to find the x-intercept
    \**********************************************************************/


    //Setup Poly Matrix
    for (workingPoly = 0; workingPoly < totalPolys; workingPoly++)  //Go through Poly's 1 by one
    {
        for (int i = 0; i < subWidth_1 * subHeight_1 * 3; i++)
            XY_Rast[i] = 1.0f;  //Set values for initial temp
        for (int i = 0; i < subWidth_2 * subHeight_2 * 3; i++)
            XZ_Rast[i] = 1.0f;  //Set values for initial temp
        for (int i = 0; i < subWidth_3 * subHeight_3 * 3; i++)
            YZ_Rast[i] = 1.0f;  //Set values for initial temp
        
        for (int curEdge = 0; curEdge < Poly[workingPoly].totalEdges; curEdge++) //Setup the poly in our blank matrix with -1's
        {
            xy_x1 = ((Poly[workingPoly].edges[curEdge].first.x - MastMin) / (float)(MastMax - MastMin)) * (subWidth_1 / 2);
            xz_x1 = ((Poly[workingPoly].edges[curEdge].first.x - MastMin) / (float)(MastMax - MastMin)) * (subWidth_2 / 2);
            xy_y1 = ((Poly[workingPoly].edges[curEdge].first.y - MastMin) / (float)(MastMax - MastMin)) * ((subHeight_1 - 1) / 2);
            yz_y1 = ((Poly[workingPoly].edges[curEdge].first.y - MastMin) / (float)(MastMax - MastMin)) * (subWidth_3 / 2);
            xz_z1 = ((Poly[workingPoly].edges[curEdge].first.z - MastMin) / (float)(MastMax - MastMin)) * ((subHeight_2 - 1) / 2);
            yz_z1 = ((Poly[workingPoly].edges[curEdge].first.z - MastMin) / (float)(MastMax - MastMin)) * ((subHeight_3 - 1) / 2);

            
            xy_x2 = ((Poly[workingPoly].edges[curEdge].second.x - MastMin) / (float)(MastMax - MastMin)) * (subWidth_1 / 2);
            xz_x2 = ((Poly[workingPoly].edges[curEdge].second.x - MastMin) / (float)(MastMax - MastMin)) * (subWidth_2 / 2);
            xy_y2 = ((Poly[workingPoly].edges[curEdge].second.y - MastMin) / (float)(MastMax - MastMin)) * ((subHeight_1 - 1) / 2);
            yz_y2 = ((Poly[workingPoly].edges[curEdge].second.y - MastMin) / (float)(MastMax - MastMin)) * (subWidth_3 / 2);
            xz_z2 = ((Poly[workingPoly].edges[curEdge].second.z - MastMin) / (float)(MastMax - MastMin)) * ((subHeight_2 - 1) / 2);
            yz_z2 = ((Poly[workingPoly].edges[curEdge].second.z - MastMin) / (float)(MastMax - MastMin)) * ((subHeight_3 - 1) / 2);

            rasterLines(xyBuf, xy_x1, xy_y1, xy_x2, xy_y2, R, G, B); //Now window is blank with poly edge's labeled as -1's
            rasterLines(xzBuf, xz_x1, xz_z1, xz_x2, xz_z2, R, G, B);
            rasterLines(yzBuf, yz_y1, yz_z1, yz_y2, yz_z2, R, G, B);
        }

        //Now scan each line. Where we encounter a -1, add to the X intercept vector
        XY_Raster(Poly[workingPoly]);
        XZ_Raster(Poly[workingPoly]);
        YZ_Raster(Poly[workingPoly]);
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
}

void rasterLines(int buffer, float X0, float Y0, float X_End, float Y_End, int R, int G, int B)
{
    float delta_X = X_End - X0, delta_Y = Y_End - Y0, steps; //delta's for X and Y and the number of steps we take in the for loop depending on the 0 < m < 1 stuff
    float xInc, yInc, Xn = X0, Yn = Y0; //Inc is the increment in the equations for Xn+1 and Yn+1 (either 1 or +/- m)

    if (delta_Y == 0) //Slope is undefined ---- vertical line
    {
        steps = fabs(delta_X);  //Steps toi move up the line
        xInc = float(delta_X) / float(steps);   //Increasing value for X
        rastPixel(buffer, round(Xn), round(Yn), R, G, B); //Set init point
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc;
            rastPixel(buffer, fabs(round(Xn)), fabs(round(Yn)), R, G, B);
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

        rastPixel(buffer, round(Xn), round(Yn), R, G, B);//Creates initial pixel
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc; //Increment X and Y by their respective values
            Yn += yInc;
            rastPixel(buffer, fabs(round(Xn)), fabs(round(Yn)), R, G, B);
        }
    }
}

void rastPixel(int buffer, float x, float y, int R, int G, int B)
{   //Backround is Init to 1.0f = white, set pixel RGB to 0.0f for black

    //Copy paste of setPixel but for raster for the tempPoly
    if (buffer == mainBuf) //Main
    {
        cout << "Invalid buffer.";
        cin.clear();    cin.get();
    }
    else if (buffer == xyBuf) //XY
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                XY_Rast[(((int)x) * 3 + ((int)y) * subWidth_1 * 3) + step] = -1;  //Set R
            else if (step == 1)
                XY_Rast[(((int)x) * 3 + ((int)y) * subWidth_1 * 3) + step] = -1;  //Set G
            else if (step == 2)
                XY_Rast[(((int)x) * 3 + ((int)y) * subWidth_1 * 3) + step] = -1;  //Set B
        }
    }
    else if (buffer == xzBuf) //XZ
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                XZ_Rast[(((int)x) * 3 + ((int)y) * subWidth_2 * 3) + step] = -1;  //Set R
            else if (step == 1)
                XZ_Rast[(((int)x) * 3 + ((int)y) * subWidth_2 * 3) + step] = -1;  //Set G
            else if (step == 2)
                XZ_Rast[(((int)x) * 3 + ((int)y) * subWidth_2 * 3) + step] = -1;  //Set B
        }
    }
    else if (buffer == yzBuf) //YZ
    {
        for (int step = 0; step < 3; step++)
        {
            if (step == 0)
                YZ_Rast[(((int)x) * 3 + ((int)y) * subWidth_3 * 3) + step] = -1;  //Set R
            else if (step == 1)
                YZ_Rast[(((int)x) * 3 + ((int)y) * subWidth_3 * 3) + step] = -1;  //Set G
            else if (step == 2)
                YZ_Rast[(((int)x) * 3 + ((int)y) * subWidth_3 * 3) + step] = -1;  //Set B
        }
    }
    else
        ;

}

void XY_Raster(THPolygon curPoly)
{
    vector<int> *xInterc = new vector<int>[subHeight_1];
    triple<float> I1(0, 0, 0);
    triple<float> I2(0, 0, 0);

    for (int y = 0; y < subHeight_1; y++)
    {
        for (int x = 0; x < subWidth_1; x++)
        {
            if (XY_Rast[x * 3 + y * subWidth_1 * 3] == -1)
            {
                    xInterc[y].push_back(x);   
            }
        }

        if (xInterc[y].size() != 0)
        {
            for (int bStep = 0; bStep < 3; bStep++)
            {
                if (bStep == 0)
                    I1.i = PIXEL_BUFFER_XY[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_1 * 3) + bStep];  //Set R
                else if (bStep == 1)
                    I1.j = PIXEL_BUFFER_XY[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_1 * 3) + bStep];  //Set G
                else if (bStep == 2)
                    I1.k = PIXEL_BUFFER_XY[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_1 * 3) + bStep];  //Set B
            }
            for (int bStep = 0; bStep < 3; bStep++)
            {
                if (bStep == 0)
                    I2.i = PIXEL_BUFFER_XY[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_1 * 3) + bStep];  //Set R
                else if (bStep == 1)
                    I2.j = PIXEL_BUFFER_XY[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_1 * 3) + bStep];  //Set G
                else if (bStep == 2)
                    I2.k = PIXEL_BUFFER_XY[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_1 * 3) + bStep];  //Set B
            }
            fillDDA(xyBuf, xInterc[y][0], y, xInterc[y][xInterc[y].size() - 1], y, curPoly, I1, I2, 1.0f, 0.0f, 0.0f);
        }
        
    }
}

void XZ_Raster(THPolygon curPoly)
{
    vector<int> *xInterc = new vector<int>[subHeight_2];
    triple<float> I1(0, 0, 0);
    triple<float> I2(0, 0, 0);

    for (int y = 0; y < subHeight_2; y++)
    {
        for (int x = 0; x < subWidth_2; x++)
        {
            if (XZ_Rast[x * 3 + y * subWidth_2 * 3] == -1)
            {
                    xInterc[y].push_back(x);   
            }
        }

        if (xInterc[y].size() != 0)
        {
            for (int bStep = 0; bStep < 3; bStep++)
            {
                if (bStep == 0)
                    I1.i = PIXEL_BUFFER_XZ[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_2 * 3) + bStep];  //Set R
                else if (bStep == 1)
                    I1.j = PIXEL_BUFFER_XZ[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_2 * 3) + bStep];  //Set G
                else if (bStep == 2)
                    I1.k = PIXEL_BUFFER_XZ[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_2 * 3) + bStep];  //Set B
            }
            for (int bStep = 0; bStep < 3; bStep++)
            {
                if (bStep == 0)
                    I2.i = PIXEL_BUFFER_XZ[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_2 * 3) + bStep];  //Set R
                else if (bStep == 1)
                    I2.j = PIXEL_BUFFER_XZ[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_2 * 3) + bStep];  //Set G
                else if (bStep == 2)
                    I2.k = PIXEL_BUFFER_XZ[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_2 * 3) + bStep];  //Set B
            }
            fillDDA(xzBuf, xInterc[y][0], y, xInterc[y][xInterc[y].size() - 1], y, curPoly, I1, I2, 1.0f, 0.0f, 0.0f);
        }
        
    }
}

void YZ_Raster(THPolygon curPoly)
{
    vector<int> *xInterc = new vector<int>[subHeight_3];
    triple<float> I1(0, 0, 0);
    triple<float> I2(0, 0, 0);

    for (int y = 0; y < subHeight_3; y++)
    {
        for (int x = 0; x < subWidth_3; x++)
        {
            if (YZ_Rast[x * 3 + y * subWidth_3 * 3] == -1)
            {
                xInterc[y].push_back(x);
            }
        }

        if (xInterc[y].size() != 0)
        {
            for (int bStep = 0; bStep < 3; bStep++)
            {
                if (bStep == 0)
                    I1.i = PIXEL_BUFFER_YZ[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_3 * 3) + bStep];  //Set R
                else if (bStep == 1)
                    I1.j = PIXEL_BUFFER_YZ[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_3 * 3) + bStep];  //Set G
                else if (bStep == 2)
                    I1.k = PIXEL_BUFFER_YZ[(((int)xInterc[y][0]) * 3 + ((int)y) * subWidth_3 * 3) + bStep];  //Set B
            }
            for (int bStep = 0; bStep < 3; bStep++)
            {
                if (bStep == 0)
                    I2.i = PIXEL_BUFFER_YZ[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_3 * 3) + bStep];  //Set R
                else if (bStep == 1)
                    I2.j = PIXEL_BUFFER_YZ[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_3 * 3) + bStep];  //Set G
                else if (bStep == 2)
                    I2.k = PIXEL_BUFFER_YZ[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * subWidth_3 * 3) + bStep];  //Set B
            }
            fillDDA(yzBuf, xInterc[y][0], y, xInterc[y][xInterc[y].size() - 1], y, curPoly, I1, I2, 1.0f, 0.0f, 0.0f);
        }
        
    }
}

//P3 Utility
void fillDDA(int buffer, float X0, int Y0, int X_End, int Y_End, THPolygon curPoly, triple<float> I1, triple<float> I2, float R, float G, float B)
{

    float delta_X = X_End - X0, delta_Y = Y_End - Y0, steps; //delta's for X and Y and the number of steps we take in the for loop depending on the 0 < m < 1 stuff
    float xInc, yInc, Xn = X0, Yn = Y0; //Inc is the increment in the equations for Xn+1 and Yn+1 (either 1 or +/- m)

    if (delta_Y == 0) //Slope is undefined ---- vertical line
    {
        steps = fabs(delta_X);  //Steps toi move up the line
        xInc = float(delta_X) / float(steps);   //Increasing value for X
        scanIntens(buffer, round(Xn), round(Yn), curPoly, I1, I2, X0, X_End);
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc;
            scanIntens(buffer, round(Xn), round(Yn), curPoly, I1, I2, X0, X_End);
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

        scanIntens(buffer, round(Xn), round(Yn), curPoly, I1, I2, X0, X_End);
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc; //Increment X and Y by their respective values
            Yn += yInc;
            scanIntens(buffer, round(Xn), round(Yn), curPoly, I1, I2, X0, X_End);
        }
    }
}

void scanIntens(int buffer, int x, int y, THPolygon curPoly, triple<float> I1, triple<float> I2, float X0, float X_End)
{
    //(((int)x) * 3 + ((int)y) * WIDTH * 3) + step
    //If edge NOT horizontal
    //I(point) = ((y(point) - y1) / (y2 - y1)) * I2 + ((y2 - y(point)) / (y2 - y1)) * I1
    //If edge horizontal
    //I(point) = ((x(point) - x1) / (x2 - x1)) * I2 + ((x2 - x(point)) / (x2 - x1)) * I1
    //Always: y1 < y(point) < y2
    //Always: x1 < x(point) < x2
    int X1 = X0;
    int X2 = X_End;
    if (X1 > X2)
    {
        int xTemp = X2;
        X2 = X1;
        X1 = xTemp;
    }

    if (buffer == xyBuf)
    {
        R = ((x - X1) / (float)(X2 - X1)) * I2.i + ((X2 - x) / (float)(X2 - X1)) * I1.i;
        B = ((x - X1) / (float)(X2 - X1)) * I2.j + ((X2 - x) / (float)(X2 - X1)) * I1.j;
        G = ((x - X1) / (float)(X2 - X1)) * I2.k + ((X2 - x) / (float)(X2 - X1)) * I1.k;
        setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
    }
    else if (buffer == xzBuf)
    {
        R = ((x - X1) / (float)(X2 - X1)) * I2.i + ((X2 - x) / (float)(X2 - X1)) * I1.i;
        B = ((x - X1) / (float)(X2 - X1)) * I2.j + ((X2 - x) / (float)(X2 - X1)) * I1.j;
        G = ((x - X1) / (float)(X2 - X1)) * I2.k + ((X2 - x) / (float)(X2 - X1)) * I1.k;
        setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
    }
    else if (buffer == yzBuf)
    {
        R = ((x - X1) / (float)(X2 - X1)) * I2.i + ((X2 - x) / (float)(X2 - X1)) * I1.i;
        B = ((x - X1) / (float)(X2 - X1)) * I2.j + ((X2 - x) / (float)(X2 - X1)) * I1.j;
        G = ((x - X1) / (float)(X2 - X1)) * I2.k + ((X2 - x) / (float)(X2 - X1)) * I1.k;
        setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
    }
}

void newPhong()
{
    //void phong(THPolygon &curPoly, float I_amb, float I_source, int nPhong, TPnt x, TPnt f, triple<float> k_amb, triple<float> k_dif, triple<float> k_spec)
    cout << "Getting new Phong Input.\n";
    cout << "I_Ambient: "; cin >> I_amb;
    cout << "I_LightSource: "; cin >> I_source;
    cout << "Phong Const: "; cin >> nPhong;
    cout << "Light Position (x, y, z):\n";
    cout << "x: "; cin >> lightPos.x;
    cout << "y: "; cin >> lightPos.y;
    cout << "z: "; cin >> lightPos.z;
    cout << "Viewing Position (x, y, z):\n";
    cout << "x: "; cin >> viewer.x;
    cout << "y: "; cin >> viewer.y;
    cout << "z: "; cin >> viewer.z;
    cout << "k_amb, R: "; cin >> k_amb.i;
    cout << "k_amb, G: "; cin >> k_amb.j;
    cout << "k_amb, B: "; cin >> k_amb.k;
    cout << "k_diff, R: "; cin >> k_diff.i;
    cout << "k_diff, G: "; cin >> k_diff.j;
    cout << "k_diff, B: "; cin >> k_diff.k;
    cout << "k_spec, R: "; cin >> k_spec.i;
    cout << "k_spec, G: "; cin >> k_spec.j;
    cout << "k_spec, B: "; cin >> k_spec.k;
    drawFaces();
    raster();
}


//Halftoning
void halftone()
{
    //Will try implementing in Pixel Buffer XY
    //void setPixel(int Buffer, float x, float y, float R, float G, float B)
    //Im really sorry, this is gonna be MESSY code

    //XY
    for (int outerX = 0; outerX < subWidth_1; outerX += 3)
    {
        for (int outerY = 0; outerY < subHeight_1; outerY += 3)
        {
            triple<float> total(0, 0, 0);
            for (int x = 0; x < 3; x++)
            {
                for (int y = 0; y < 3; y++)
                {
                    total.i += PIXEL_BUFFER_XY[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_1 * 3) + 0];
                    total.j += PIXEL_BUFFER_XY[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_1 * 3) + 1];
                    total.k += PIXEL_BUFFER_XY[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_1 * 3) + 2];
                }
            }
            //Average out the I of this section. After averaged, setPixel of same chunk with the ONE intensity
            total.i /= 9;
            total.j /= 9;
            total.k /= 9;
            for (int x = 0; x < 3; x++)
            {
                for (int y = 0; y < 3; y++)
                {
                    setPixel(xyBuf, x + outerX, y + outerY, total.i, total.j, total.k);
                }
            }
        }
    }

    //XZ
    for (int outerX = 0; outerX < subWidth_2; outerX += 3)
    {
        for (int outerY = 0; outerY < subHeight_2; outerY += 3)
        {
            triple<float> total(0, 0, 0);
            for (int x = 0; x < 3; x++)
            {
                for (int y = 0; y < 3; y++)
                {
                    total.i += PIXEL_BUFFER_XZ[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_2 * 3) + 0];
                    total.j += PIXEL_BUFFER_XZ[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_2 * 3) + 1];
                    total.k += PIXEL_BUFFER_XZ[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_2 * 3) + 2];
                }
            }
            //Average out the I of this section. After averaged, setPixel of same chunk with the ONE intensity
            total.i /= 9;
            total.j /= 9;
            total.k /= 9;
            for (int x = 0; x < 3; x++)
            {
                for (int y = 0; y < 3; y++)
                {
                    setPixel(xzBuf, x + outerX, y + outerY, total.i, total.j, total.k);
                }
            }
        }
    }

    //YZ
    for (int outerX = 0; outerX < subWidth_3; outerX += 3)
    {
        for (int outerY = 0; outerY < subHeight_3; outerY += 3)
        {
            triple<float> total(0, 0, 0);
            for (int x = 0; x < 3; x++)
            {
                for (int y = 0; y < 3; y++)
                {
                    total.i += PIXEL_BUFFER_YZ[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_3 * 3) + 0];
                    total.j += PIXEL_BUFFER_YZ[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_3 * 3) + 1];
                    total.k += PIXEL_BUFFER_YZ[(((int)x + outerX) * 3 + ((int)y + outerY) * subWidth_3 * 3) + 2];
                }
            }
            //Average out the I of this section. After averaged, setPixel of same chunk with the ONE intensity
            total.i /= 9;
            total.j /= 9;
            total.k /= 9;
            for (int x = 0; x < 3; x++)
            {
                for (int y = 0; y < 3; y++)
                {
                    setPixel(yzBuf, x + outerX, y + outerY, total.i, total.j, total.k);
                }
            }
        }
    }
}
//New rasterizer
/********************************************************\
    Need to rasterize surface by surface. Best way could
    be drawing polygon in a new pixel buffer, one by one
    and rasterizing surface by surface.


\********************************************************/

//Pipeline
/********************************************************\
    Phong first.
    DDA with modification for color.
    Start sorting surfaces for hidden surfaces.


    Michael must be able to input any of the variables
    in the phong equationg (except c).
    Also coordinates of the light source (one point)

    Halftoning, whether to trigger pixel or not.

    End result, intensity from 0-9.
    Need to go from 0-1 to 0-9 - MEGAPIXELS.
    One pixel becomes a 3x3 "pixel" so original reso
    of 100x100 becomes 300x300

\********************************************************/

/********************************************************\
3 - number of objects

4 - number of vertices
1.0 0.0 0.0 - vertex 1
0.0 1.0 0.0
2.0 2.0 0.0
1.0 1.0 2.0
n   n   n   - vertex 1 normal
n   n   n   - vertex 2 normal
n   n   n
n   n   n
6 - number of edges
1 2 - edge 1-2
1 3
1 4
2 3
2 4
3 4
4 - number of surfaces
1 2 3
1 2 4
1 3 4
2 3 4
0.0 0.0 -3.0 - normals for each surface
-2.0 -2.0 1.0
4.0 -2.0 1.0
-2.0 4.0 1.0
\********************************************************/
