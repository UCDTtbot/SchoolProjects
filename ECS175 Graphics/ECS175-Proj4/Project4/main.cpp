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
#include <map>
#include <utility>
#include "2DPoint.h"
#include "BCurve.h"

/****************************/
//							//
//		Using main.cpp		//
//		From Project 2		//
//							//
/****************************/
using namespace std;
    //Variables

//Window and PixelBuffer declares
float *PIXEL_BUFFER;
static int CURRENT_CURVE = 0;
static int MainWindow;
static int WIDTH = 800, HEIGHT = 600;
//Master Max and Min for normalization
static int MastMax = 0, MastMin = 0;

//RGB declares and converter
float RGB_CONV = 1/(float)255;
float R = 0.0f, G = 0.0f, B = 0.0f;

//NDC
static vector<pair<Pnt, Pnt> > NDC;

//Menu Declares
static int menuID, subMenuID1, subMenuID2;
static int menuValue;

//File Declares
string fileName = "POLYS.dat";
ofstream outputFile;
ifstream inputFile;
void loadFile();
void saveFile();

//Radian to Degree conversions
const float  PI_F = 3.14159265358979f; //PI for degrees to radians
const float D_TO_R = PI_F / 180;    //Use to convert degrees_to_radian

//Line Declares
static int totalCurves = 0;
static int maxCurves = 4;
static int RES = 10;
vector<BCurve> Curves;
vector<Pnt> cPnts;

    //Functions

//OpenGL Functions
void main_display();
void init();
void createMenu();
void menu(int num);

//Line Drawing Algorithms
void setPixel(float x, float y, float R, float G, float B);
void lineDDA(float X0, float Y0, float X, float Y, float R, float G, float B);
void drawLines();
void drawCurves();
void setupControlPoly();

//Normalization
void getMasters();
void normalize(float&, float&, float&, float&);

//Utility
void displayCurveInfo(int curveID);
void getFileName();
void redisplay();
void selectCurve();
void animate();
void createEdges(BCurve &curve);
void checkKnots(BCurve &curve);

int main(int argc, char* argv[])
{
    //Load File
    cout << "Welcome.\n";
    getFileName();
    loadFile();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowPosition(250, 100);
    glutInitWindowSize(WIDTH, HEIGHT);

    MainWindow = glutCreateWindow("Project 4");
    glutDisplayFunc(main_display);

    createMenu();
    init();

    //display current curve
    displayCurveInfo(CURRENT_CURVE);
    //for (int step = 0; step < totalCurves; step++)
    //    displayCurveInfo(step);
    glutMainLoop();

    return 0;
}

//Initialize Window and Variables
void init()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

    PIXEL_BUFFER = new float [WIDTH * HEIGHT * 3];

    for(int step = 0; step < WIDTH * HEIGHT * 3; step++)
        PIXEL_BUFFER[step] = 1.0f;

    for (int step = 0; step < totalCurves; step++)
    {
        pair<Pnt, Pnt> *newPnt = new pair<Pnt, Pnt>;

        NDC.push_back(*newPnt);
    }

    //drawLines();
    drawCurves();
    //setupControlPoly();

    glutSetWindow(MainWindow);

}

void setPixel(float x, float y, float R, float G, float B)
{
    for (int step = 0; step < 3; step++)
    {
        if (step == 0)
            PIXEL_BUFFER[(((int)round(x)) * 3 + ((int)round(y)) * WIDTH * 3) + step] = R;  //Set R
        else if (step == 1)
            PIXEL_BUFFER[(((int)round(x)) * 3 + ((int)round(y)) * WIDTH * 3) + step] = G;  //Set G
        else if (step == 2)
            PIXEL_BUFFER[(((int)round(x)) * 3 + ((int)round(y)) * WIDTH * 3) + step] = B;  //Set B
    }
}

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
            steps = round(fabs(delta_Y));
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

void main_display()
{
    //Function that is looped through
    glutSetWindow(MainWindow);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    //Draws the PixelBuffer to screen - reword for subwindows
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, PIXEL_BUFFER);

    glFlush();
}

void createMenu()
{
    //First create submenu
    glutSetWindow(MainWindow);
    subMenuID1 = glutCreateMenu(menu);  //SubMenu for Polygon IDs
    glutAddMenuEntry("Curve #1", 1);
    glutAddMenuEntry("Curve #2", 2);
    glutAddMenuEntry("Curve #3", 3);
    glutAddMenuEntry("Curve #4", 4);
    glutAddMenuEntry("Curve #5", 5);
    glutAddMenuEntry("Curve #6", 6);
    subMenuID2 = glutCreateMenu(menu);  //SubMenu for Manipulate
    glutAddMenuEntry("Add Control Point", 7);
    glutAddMenuEntry("Delete Control Point", 8);
    glutAddMenuEntry("Change Control Point", 9);
    glutAddMenuEntry("Change K value", 10);
    glutAddMenuEntry("Add Knot", 11);
    glutAddMenuEntry("Delete Knot", 12);
    glutAddMenuEntry("Change Knot", 16);
    menuID = glutCreateMenu(menu);  //Setup main Menu
    glutAddSubMenu("Select Polygon", subMenuID1);
    glutAddSubMenu("Manipulate", subMenuID2);   //Attaches to subMenu 2
    glutAddMenuEntry("Display...", 13);
    glutAddMenuEntry("Save", 14);
    glutAddMenuEntry("Load", 15);
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
        if (totalCurves < 1)
            cout << "Curve does not exist.";
        else
        {
            CURRENT_CURVE = 0;
            cout << "\nSelected Curve " << CURRENT_CURVE + 1 << endl;
            displayCurveInfo(CURRENT_CURVE);
        }
    }
    else if (num == 2)  //Polygon #2
    {
        if (totalCurves < 2)
            cout << "Polygon does not exist.";
        else
        {
            CURRENT_CURVE = 1;
            cout << "\nSelected Curve " << CURRENT_CURVE + 1 << endl;
            displayCurveInfo(CURRENT_CURVE);
        }
    }
    else if (num == 3)  //Polygon #3
    {
        if (totalCurves < 3)
            cout << "Polygon does not exist.";
        else
        {
            CURRENT_CURVE = 2;
            cout << "\nSelected Curve " << CURRENT_CURVE + 1 << endl;
            displayCurveInfo(CURRENT_CURVE);
        }
    }
    else if (num == 4) //Polygon 4
    {
        if (totalCurves < 4)
            cout << "Polygon does not exist.";
        else
        {
            CURRENT_CURVE = 3;
            cout << "\nSelected Curve " << CURRENT_CURVE + 1 << endl;
            displayCurveInfo(CURRENT_CURVE);
        }
    }
    else if (num == 5) //Polygon 5
    {
        if (totalCurves < 5)
            cout << "Polygon does not exist.\n";
        else
        {
            CURRENT_CURVE = 4;
            cout << "\nSelected Curve " << CURRENT_CURVE + 1 << endl;
            displayCurveInfo(CURRENT_CURVE);
        }
    }
    else if (num == 6) //Polygon 6
    {
        if (totalCurves < 6)
            cout << "Polygon does not exist.\n";
        else
        {
            CURRENT_CURVE = 5;
            cout << "\nSelected Curve " << CURRENT_CURVE + 1 << endl;
            displayCurveInfo(CURRENT_CURVE);
        }
    }
    else if (num == 7)  //Add Control Point
    {
        BCurve curCurve = Curves[CURRENT_CURVE];
        int pos, x, y;
        cout << "There are currently " << curCurve.totalPoints << " control points in Curve # " << CURRENT_CURVE + 1 << endl;
        cout << "Please enter the position you would like to enter a control point from 0 to " << curCurve.totalPoints - 1 << endl;
        cout << "Please note, if a value already exists at the position, points will be pushed back" << endl;
        cin >> pos;
        cout << "Please enter the X value: "; cin >> x;
        cout << "Please enter the Y value: "; cin >> y;
        cout << "Adding point (" << x << ", " << y << ") in position " << pos << endl;
        vector<Pnt>::iterator IT = curCurve.controlPoints.begin() + pos;
        curCurve.controlPoints.insert(IT, Pnt(x, y));
        curCurve.totalPoints++;
        createEdges(curCurve);
        Curves[CURRENT_CURVE] = curCurve;
        drawCurves();
    }
    else if (num == 8)  //Delete Control Point
    {
        BCurve curCurve = Curves[CURRENT_CURVE];
        int pos, x, y;
        cout << "There are currently " << curCurve.totalPoints << " control points in Curve # " << CURRENT_CURVE + 1 << endl;
        cout << "Please enter the position you would like to delete a control point from 0 to " << curCurve.totalPoints - 1 << endl;
        cin >> pos;
        vector<Pnt>::iterator IT = curCurve.controlPoints.begin() + pos;
        cout << "Deleting point (" << (*IT).x << ", " << (*IT).y << ") in position " << pos << endl;
        curCurve.controlPoints.erase(IT);
        curCurve.totalPoints--;
        createEdges(curCurve);
        Curves[CURRENT_CURVE] = curCurve;
        drawCurves();
    }
    else if (num == 9)  //Change Control Point
    {
        BCurve curCurve = Curves[CURRENT_CURVE];
        int pos, x, y;
        cout << "There are currently " << curCurve.totalPoints << " control points in Curve # " << CURRENT_CURVE + 1 << endl;
        cout << "Please enter the position you would like to change a control point from 0 to " << curCurve.totalPoints - 1 << endl;
        cin >> pos;
        cout << "Please enter the X value: "; cin >> x;
        cout << "Please enter the Y value: "; cin >> y;
        vector<Pnt>::iterator IT = curCurve.controlPoints.begin() + pos;
        cout << "Changing point (" << (*IT).x << ", " << (*IT).y << ") in position " << pos << " to (" << x << ", " << y << ")" << endl;
        curCurve.controlPoints[pos].x = x;
        curCurve.controlPoints[pos].y = y;
        Curves[CURRENT_CURVE] = curCurve;
        drawCurves();
    }
    else if (num == 10) //Change K Value
    {
        BCurve curCurve = Curves[CURRENT_CURVE];
        if (curCurve.TYPE.compare("Bs") != 0)
        {
            cout << "Curve is not a BSpline.\n";
        }
        else
        {
            int newK;
            cout << "The current k value is: " << curCurve.kValue << endl;
            cout << "Please enter the value to change to: ";
            cin >> newK;
            cout << "Changing K value to: " << newK << endl;
            curCurve.kValue = newK;
        }
        checkKnots(curCurve);
        Curves[CURRENT_CURVE] = curCurve;
        drawCurves();
    }
    else if (num == 11) //Add Knot
    {
        BCurve curCurve = Curves[CURRENT_CURVE];
        if (curCurve.TYPE.compare("Bs") != 0)
        {
            cout << "Curve is not a BSpline.\n";
        }
        else
        {
            int pos, newKnot;
            cout << "There are currently " << curCurve.numKnots << " of knots in Curve # " << CURRENT_CURVE + 1 << endl;
            cout << "Please enter the position you would like to enter a knot from 0 to " << curCurve.numKnots - 1 << endl;
            cout << "Please note, if a value already exists at the position, points will be pushed back" << endl;
            cin >> pos;
            if (pos > curCurve.kValue + curCurve.totalPoints)
                cout << "Too many knots, please try after deleting.\n";
            else
            {
                cout << "Please the knot value "; cin >> newKnot;
                cout << "Adding knot" << newKnot << " at position " << pos << endl;
                vector<int>::iterator IT = curCurve.knots.begin() + pos;
                curCurve.knots.insert(IT, newKnot);
                curCurve.numKnots++;
            }
        }
        void checkKnots(BCurve &curve);
        Curves[CURRENT_CURVE] = curCurve;
        drawCurves();
    }
    else if (num == 12) //Delete Knot
    {
        BCurve curCurve = Curves[CURRENT_CURVE];
        if (curCurve.TYPE.compare("Bs") != 0)
        {
            cout << "Curve is not a BSpline.\n";
        }
        else
        {
            BCurve curCurve = Curves[CURRENT_CURVE];
            int pos;
            cout << "There are currently " << curCurve.numKnots << " of knots in Curve # " << CURRENT_CURVE + 1 << endl;
            cout << "Please enter the position you would like to delete a knot from 0 to " << curCurve.numKnots - 1 << endl;
            cin >> pos;
            vector<int>::iterator IT = curCurve.knots.begin() + pos;
            cout << "Deleting knot " << *IT << ") in position " << pos << endl;
            curCurve.knots.erase(IT);
            curCurve.numKnots--;
        }
        void checkKnots(BCurve &curve);
        Curves[CURRENT_CURVE] = curCurve;
        drawCurves();
    }
    else if (num == 16) //Change Knot
    {
        BCurve curCurve = Curves[CURRENT_CURVE];
        if (curCurve.TYPE.compare("Bs") != 0)
        {
            cout << "Curve is not a BSpline.\n";
        }
        else
        {
            int pos, newKnot;
            cout << "There are currently " << curCurve.numKnots << " of knots in Curve # " << CURRENT_CURVE + 1 << endl;
            cout << "Please enter the position you would like to change a knot from 0 to " << curCurve.numKnots - 1 << endl;
            cin >> pos;
            cout << "Please the knot value "; cin >> newKnot;
            vector<int>::iterator IT = curCurve.knots.begin() + pos;
            cout << "Changing knot " << *IT <<  " in position " << pos << " to " << newKnot << endl;
            curCurve.knots[pos] = newKnot;
        }
        Curves[CURRENT_CURVE] = curCurve;
        drawCurves();
    }
    else if (num == 13)  //Display
    {
        cout << endl;
        displayCurveInfo(CURRENT_CURVE);
    }
    else if (num == 14)  //Save
    {
        saveFile();
    }
    else if (num == 15) //Load
    {
        Curves.clear();
        loadFile();
        init();
    }
    else //Fallback function
    {
        menuValue = num;
    }

    redisplay();  //One call to postRedisplay all windows
}

void redisplay()
{
    glutSetWindow(MainWindow);
    glutPostRedisplay(); //Repost the display function to redraw everything
}

void drawLines()  //drawLine for buffer
{
    //Reset Buffers
    for (int step = 0; step < WIDTH * HEIGHT * 3; step++)//Set initial buffer to a blank white backround
        PIXEL_BUFFER[step] = 1.0f;

    for (int pStep = 0; pStep < totalCurves; pStep++)   //For Each Poly and For each edge IN that poly
    {
        BCurve curCurve = Curves[pStep];

        for (int step = 0; step < curCurve.totalPoints; step++)
        {
            //normalize(pStep, step, curCurve);
            //lineDDA(NDC[pStep].first.x, NDC[pStep].first.y, NDC[pStep].second.x, NDC[pStep].second.y, 1.0f, 0.0f, 0.0f);
        }
    }
}

void drawCurves() //Draw all curves
{
    //Clear buffer
    for (int step = 0; step < WIDTH * HEIGHT * 3; step++)
        PIXEL_BUFFER[step] = 1.0f;
    getMasters();
    setupControlPoly();
    for (int cStep = 0; cStep < totalCurves; cStep++)
    {
        BCurve curCurve = Curves[cStep];
        int n = Curves[cStep].totalPoints;
        map<pair<int, int>, Pnt> b;
        map<pair<int, int>, Pnt> d;
        for (int step = 0; step < n; step++)
        {
            b[make_pair(0, step)].x = curCurve.controlPoints[step].x;
            b[make_pair(0, step)].y = curCurve.controlPoints[step].y;
        }
        cPnts.clear();
        
        if (curCurve.TYPE.compare("Bz") == 0)//Bez
        {
            //for(j = 1 TO num_of_controlPoints)
            //  for(i = 0 TO n - j)
            //      b(j,i) = (1 - t)*b(j-1, i) + t*b(j-1, i+1)
            //return b(0,n)
            float tStep = 1.0f / (float)(RES - 1);
            for (float t = 0.0f; t <= 1; t += tStep) // for each t, resolution steps
            {
                for (int j = 1; j < n; j++)
                {
                    for (int i = 0; i < n - j; i++)
                    {
                        b[make_pair(j, i)].x = (1 - t)*b[make_pair(j - 1, i)].x + t*b[make_pair(j - 1, i + 1)].x;
                        b[make_pair(j, i)].y = (1 - t)*b[make_pair(j - 1, i)].y + t*b[make_pair(j - 1, i + 1)].y;
                    }
                }

                cPnts.push_back(b[make_pair(n - 1, 0)]);
            }
            //cout << "Curve Point, t =  " << t << ": ";
            //cout << "(" << cPnts.back().x << ", " << cPnts.back().y << ")\n";
        }
        else if (curCurve.TYPE.compare("Bs") == 0)//Bs
        {
            vector<int> knots;
            int k = curCurve.kValue;
            //int step = 0;
            //for (vector<int>::iterator IT = curCurve.knots.begin(); IT != curCurve.knots.end(); IT++)
            //{
            //    knots[step] = (*IT);
            //    step++;
            //}
            for (int step = 0; step < curCurve.numKnots; step++)
            {
                knots.push_back(curCurve.knots[step]);
            }
            float uInc = ((knots[n + 1] - knots[k - 1]) / (float)(RES - 1));
            for (float u = k - 1; u < n ; u += uInc)
            {
                //Note u is Ubar
                //for(j = 1) to (k - 1) do
                //  for(i = (I - (k - 1)) to (I - j) do
                //      d(j,i) = ((Ui+k - Ubar)/(Ui+k - Ui+j))*d(j-1, i) + ((Ubar + Ui+j) / (Ui+k - Ui+j))*d(j+1, i+1)
                //return d(k-1, I-(k-1))
                int I = (int)u;
                for (int j = 1; j <= (k - 1); j++)
                {
                    for (int i = (I - (k - 1)); i <= I - j; i++)
                    {
                        float num1 = knots[i + k] - u;
                        float num2 = u - knots[i + j];
                        float denom = knots[i + k] - knots[i + j];
                        float pnt1X = b[make_pair(j - 1, i)].x;
                        float pnt1Y = b[make_pair(j - 1, i)].y;
                        float pnt2X = b[make_pair(j - 1, i + 1)].x;
                        float pnt2Y = b[make_pair(j - 1, i + 1)].y;

                        b[make_pair(j, i)].x = (num1 / denom)*pnt1X + (num2 / denom)*pnt2X;
                        b[make_pair(j, i)].y = (num1 / denom)*pnt1Y + (num2 / denom)*pnt2Y;
                        
                    }
                }
                cPnts.push_back(b[make_pair(k-1, I-(k-1))]);
                //cout << "Curve Point, u =  " << u << ": ";
                //cout << "(" << cPnts.back().x << ", " << cPnts.back().y << ")\n";
            }
        }
        else
        {
            cout << "Incorrect Curve Type in drawCurves(). Press Enter to exit.";
            cin.clear();
            cin.get();
            exit(EXIT_FAILURE);
        }
        
        //cout << "Drawing Line of curve between points, after NDC: \n";
        int pointCnt = 0;
        for (vector<Pnt>::iterator IT = cPnts.begin(); IT != cPnts.end() - 1; IT++)
        {
            float x0, y0, x, y;
            x0 = (*IT).x;
            y0 = (*IT).y;
            x = (*(IT + 1)).x;
            y = (*(IT + 1)).y;
            normalize(x0, y0, x, y);
            //cout << "Point # " << pointCnt << ": (" << x0 << ", " << y0 << ")\n";  pointCnt++;
            lineDDA(x0, y0, x, y, 0.0f, 0.0f, 0.0f);
        }
    }
    cout << endl;
}

void setupControlPoly()
{
    getMasters();
    for (int cStep = 0; cStep < totalCurves; cStep++)
    {
        BCurve curCurve = Curves[cStep];
        for (int eStep = 0; eStep < curCurve.totalEdges; eStep++)
        {
            float x0 = curCurve.controlEdges[eStep].first.x;
            float y0 = curCurve.controlEdges[eStep].first.y;
            float x = curCurve.controlEdges[eStep].second.x;
            float y = curCurve.controlEdges[eStep].second.y;
            normalize(x0, y0, x, y);
            lineDDA(x0, y0, x, y, 0.0f, 0.8f, 0.0f);
        }
    }
}

void normalize(float &x1, float &y1, float &x2, float &y2)
{
    float masterMax = MastMax, masterMin = MastMin;
    //(point - masterMin) / (masterMax - masterMin)
    x1 = ((x1 - masterMin) / (float)(masterMax - masterMin)) * (WIDTH);
    y1 = ((y1 - masterMin) / (float)(masterMax - masterMin)) * ((HEIGHT - 1));
    x2 = ((x2 - masterMin) / (float)(masterMax - masterMin)) * (WIDTH);
    y2 = ((y2 - masterMin) / (float)(masterMax - masterMin)) * ((HEIGHT - 1));

//    NDC[cStep].first.x = X1 * (WIDTH / 2); NDC[cStep].first.y = Y1 * ((HEIGHT - 1) / 2);
//    NDC[cStep].second.x = X2 * (WIDTH / 2); NDC[cStep].second.y = Y2 * ((HEIGHT - 1) / 2);
}

void getMasters()
{
    //Normalize every vert from world coords to [0.0, 1.0]
    //Get MasterMax and MasterMin
    float x, y;
    BCurve curCurve;
    for (int cStep = 0; cStep < totalCurves; cStep++)
    {
        curCurve = Curves[cStep];
        for (int step = 0; step < curCurve.totalPoints; step++)
        {
            x = curCurve.controlPoints[step].x;
            y = curCurve.controlPoints[step].y;

        //    //MastMax
            if (x > MastMax)
                MastMax = x;
            if (y > MastMax)
                MastMax = y;

        //    //MastMin
            if (x < MastMin)
                MastMin = x;
            if (y < MastMin)
                MastMin = y;
        }
    }
    //Normalize a coordinate of x y by ( (x - mastMin) / (mastMax - mastMin), (y - mastMin) / (mastMax - mastMin) )
}

void loadFile()
{
    //Files will have very strict formatting. Will crash unless its very specifically formatted
    ifstream inFile;
    inFile.open(fileName.c_str());
    string curveType;
    int bz = 0, bs = 0;
    int totalPoints = 0;
    int resolution;
    int pnt1, pnt2;
    int pntPos = 0;
    float x, y;
    inFile >> resolution;
    RES = resolution;
    inFile >> totalCurves;   //Read total Polys


    if (totalCurves <= maxCurves && totalCurves >= 0)
    {
        for (int step = 0; step < totalCurves; step++)
        {
            inFile >> curveType;
            bz = 0; bs = 0;
            if (curveType.compare("Bz") == 0)
                bz = 1;
            else if (curveType.compare("Bs") == 0)
                bs = 1;
            else
            {
                cout << "Error, invalid curve specifier in loadFile(). Press Enter to exit.\n";
                cin.clear();
                cin.get();
                exit(EXIT_FAILURE);
            }

            if (bz)
            {

                BCurve newCurve(step, "Bz");
                inFile >> totalPoints;
                for (int pStep = 0; pStep < totalPoints; pStep++)
                {
                    inFile >> x; inFile >> y;
                    newCurve.addPoints(x, y);
                }
                createEdges(newCurve);
                Curves.push_back(newCurve);
                
            }
            else if (bs)
            {
                int k;
                BCurve newCurve(step, "Bs");
                bool knots = false;
                inFile >> totalPoints;
                inFile >> k;
                newCurve.kValue = k;
                inFile >> knots;
                newCurve.knotBool = knots;
                newCurve.numKnots = totalPoints + k + 1;
                for (int pStep = 0; pStep < totalPoints; pStep++)
                {
                    inFile >> x; inFile >> y;
                    newCurve.addPoints(x, y);
                }
                createEdges(newCurve);
                if (knots)
                {
                    for (int kStep = 0; kStep < newCurve.numKnots; kStep++)
                    {
                        int knV;
                        inFile >> knV;
                        newCurve.knots.push_back(knV);
                    }
                }
                else
                {
                    for (int kStep = 0; kStep < newCurve.numKnots; kStep++)
                    {
                        newCurve.knots.push_back(kStep);
                    }
                }
                Curves.push_back(newCurve);
            }
            else
            {
                cout << "Incorrect Curve Type in loadFile(). Press Enter to exit.";
                cin.clear();
                cin.get();
                exit(EXIT_FAILURE);
            }
        }
    }
    else
        cout << "Too many or too few curves in file. Please double check.\nShould be between 0 and 6.\n";
    inFile.close();
    //Everything should be set just as if user was entering info into the console
}

void createEdges(BCurve &curve)
{
    int pntPos = 0;
    curve.controlEdges.clear();
    curve.totalEdges = 0;
    for (vector<Pnt>::iterator IT = curve.controlPoints.begin(); IT != curve.controlPoints.end() - 1; IT += 1)
    {
        curve.addEdge((*IT), (*(IT + 1)), pntPos, pntPos + 1);
        pntPos += 1;
    }
}

void checkKnots(BCurve &curve)
{
    if (curve.numKnots > curve.totalPoints + curve.kValue)
    {
        cout << "In function checkKnots(BCurve&). There exists too many knots. Deleting last knot.\n";
        curve.knots.erase(curve.knots.end() - 1);
        curve.numKnots--;
    }
}


void saveFile()
{
    ofstream outFile;
    string Bz = "Bz", Bs = "Bs";
    outFile.open(fileName.c_str(), ios::trunc);
    outFile << RES << endl;
    outFile << totalCurves << endl;
    for (int step = 0; step < totalCurves; step++)
    {
        if (Curves[step].TYPE.compare(Bz) == 0)//Bezier
        {
            outFile << Bz << endl;
            outFile << Curves[step].totalPoints << endl;
            for (int Pstep = 0; Pstep < Curves[step].totalPoints; Pstep++)
            {
                outFile << Curves[step].controlPoints[Pstep].x << " " << Curves[step].controlPoints[Pstep].y << endl;
            }

        }
        else if (Curves[step].TYPE.compare(Bs) == 0) //BSpline
        {
            outFile << Bs << endl;
            outFile << Curves[step].totalPoints << endl;
            outFile << Curves[step].kValue << endl;
            outFile << Curves[step].knotBool << endl;
            for (int Pstep = 0; Pstep < Curves[step].totalPoints; Pstep++)
            {
                outFile << Curves[step].controlPoints[Pstep].x << " " << Curves[step].controlPoints[Pstep].y << endl;
            }
            if (Curves[step].knotBool)
            {
                for (int kStep = 0; kStep < Curves[step].knots.size(); kStep++)
                    outFile << Curves[step].knots[kStep] << " ";
                outFile << endl;
            }
        }
        else
        {
            cout << "Incorrect curve type in saveFile(). Press Enter to exit.";
            cin.clear();
            cin.get();
            exit(EXIT_FAILURE);
        }
    }
    outFile.close(); 
}

void getFileName()
{
    cout << "Please enter file name. Enter d for default name (CURVE.dat): \n";
    cin >> fileName;
    if (fileName.compare("d") == 0 || fileName.compare("D") == 0)
        fileName = "CURVE.dat";
    cout << "Loaded file: " << fileName << endl;

}

void displayCurveInfo(int PolyID)
{
    BCurve curCurve = Curves[PolyID];

    cout << "Current Curve: " << PolyID + 1 << endl;
    cout << "Curve Type: " << curCurve.TYPE << endl;
    cout << "Points for curve #" << PolyID << " are: \n";
    for (int pStep = 0; pStep < curCurve.totalPoints; pStep++)
    {
        cout << curCurve.controlPoints[pStep].x << " " << curCurve.controlPoints[pStep].y << "\n";
    }
    cout << "Edges for curve #" << PolyID << " are between points: \n";
    for (int eStep = 0; eStep < curCurve.totalEdges; eStep++)
    {
        cout << curCurve.controlEdges[eStep].fstCoord << " " << curCurve.controlEdges[eStep].scdCoord << "\n";
    }
    if (curCurve.TYPE.compare("Bs") == 0)
    {
        cout << "Does curve have defined knots: " << curCurve.knotBool << endl;
        cout << "Num of Knots: " << curCurve.numKnots << endl;
        if (curCurve.knotBool)
        {
            cout << "Defined Knots: ";
            for (int kStep = 0; kStep < curCurve.knots.size(); kStep++)
                cout << curCurve.knots[kStep] << " ";
            cout << "\n";
        }
    }
}

/*
File Format:
    Resolution
    #Of Curves
    Type_Of_Curve
    #Of Points
    Points
    ....
    Type_Of_Curve
    #Of Points
    Points
    ....
    _IF_CURVE_TYPE_IS_BS_
    Bs
    #Of Points
    K Value
    Knots Defined? (T|F)
    Points
    ....
    Knot Values

*/















