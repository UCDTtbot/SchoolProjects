#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <utility>
#include <math.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include "3DPolygon.h"


/****************************/
//							//
//		Using main.cpp		//
//		From Project 3		//
//							//
/****************************/
    //Variables
//Window and PixelBuffer declares
float *PIXEL_BUFFER;
static int CURRENT_POLY = 0;
static int MainWindow;
static int WIDTH = 800, HEIGHT = 800;
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
static int totalPoly = 0;
static int maxPoly = 4;
static int RES = 10;
vector<THPolygon> Polys;
    
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

//Normalization
void getMasters();
void normalize(float&, float&, float&, float&);

//Utility
void displayPolyInfo(int PolyID);
void getFileName();
void redisplay();
void selectPoly();

using namespace std;

//Main, glutLoop, and initial load functions
int main(int argc, char* argv[])
{
    //Load File
    std::cout << "Welcome.\n";
    getFileName();
    loadFile();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowPosition(250, 100);
    glutInitWindowSize(WIDTH, HEIGHT);

    MainWindow = glutCreateWindow("Project 5");
    glutDisplayFunc(main_display);

    createMenu();
    init();

    //display current curve
    displayPolyInfo(CURRENT_POLY);
    glutMainLoop();

    return 0;
}

void init()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

    PIXEL_BUFFER = new float [WIDTH * HEIGHT * 3];

    for(int step = 0; step < WIDTH * HEIGHT * 3; step++)
        PIXEL_BUFFER[step] = 1.0f;

    for (int step = 0; step < totalPoly; step++)
    {
        pair<Pnt, Pnt> *newPnt = new pair<Pnt, Pnt>;

        NDC.push_back(*newPnt);
    }

    glutSetWindow(MainWindow);

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

//Menu Functions
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
        if (totalPoly < 1)
            std::cout << "Curve does not exist.";
        else
        {
            CURRENT_POLY = 0;
            std::cout << "\nSelected Curve " << CURRENT_POLY + 1 << endl;
            displayPolyInfo(CURRENT_POLY);
        }
    }
    else if (num == 2)  //Polygon #2
    {
        if (totalPoly < 2)
            std::cout << "Polygon does not exist.";
        else
        {
            CURRENT_POLY = 1;
            std::cout << "\nSelected Curve " << CURRENT_POLY + 1 << endl;
            displayPolyInfo(CURRENT_POLY);
        }
    }
    else if (num == 3)  //Polygon #3
    {
        if (totalPoly < 3)
            std::cout << "Polygon does not exist.";
        else
        {
            CURRENT_POLY = 2;
            std::cout << "\nSelected Curve " << CURRENT_POLY + 1 << endl;
            displayPolyInfo(CURRENT_POLY);
        }
    }
    else if (num == 7)  //Add Control Point
    {

    }
    else if (num == 8)  //Delete Control Point
    {

    }
    else if (num == 9)  //Change Control Point
    {

    }
    else if (num == 10) //Change K Value
    {

    }
    else if (num == 11) //Add Knot
    {

    }
    else if (num == 12) //Delete Knot
    {

    }
    else if (num == 16) //Change Knot
    {

    }
    else if (num == 13)  //Display
    {
        std::cout << endl;
        displayPolyInfo(CURRENT_POLY);
    }
    else if (num == 14)  //Save
    {
        saveFile();
    }
    else if (num == 15) //Load
    {
        Polys.clear();
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

//Line Drawing
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

void drawLines(int pStep, vector<Face> sortedFaces)
{

    THPolygon curPoly = Polys[pStep];
    //void phong(THPolygon curPoly, float I_amb, float I_source, int nPhong, TPnt x (light source), TPnt f (viewing vector), triple k_amb, triple k_dif, triple k_spec)
    //default datas, TODO: ADD ACTUAL PARAMS
    //phong(curPoly, I_amb, I_source, nPhong, lightPos, viewer, k_amb, k_diff, k_spec);



    for (vector<Face>::iterator it = sortedFaces.end() - 1; it != sortedFaces.begin(); --it)
    {
        for (int eStep = 0; eStep < 3; eStep++)
        {
            //normalize(pStep, eStep, *it);
            //lineDDA(xyBuf, NDC_XY[pStep].first.x, NDC_XY[pStep].first.y, NDC_XY[pStep].second.x, NDC_XY[pStep].second.y, curPoly, eStep, 0.0f, 0.0f, 0.0f);
            //lineDDA(xzBuf, NDC_XZ[pStep].first.x, NDC_XZ[pStep].first.z, NDC_XZ[pStep].second.x, NDC_XZ[pStep].second.z, curPoly, eStep, 0.0f, 0.0f, 0.0f);
            //lineDDA(yzBuf, NDC_YZ[pStep].first.y, NDC_YZ[pStep].first.z, NDC_YZ[pStep].second.y, NDC_YZ[pStep].second.z, curPoly, eStep, 0.0f, 0.0f, 0.0f);
        }
    }
    

    //P3 Calls
    //P3 Pipeline goes here
}

//Ray Tracing

//File I/O 
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
    inFile >> totalPoly;   //Read total Polys


    if (totalPoly <= maxPoly && totalPoly >= 0)
    {
        for (int step = 0; step < totalPoly; step++)
        {


        }
    }
    else
        std::cout << "Too many or too few curves in file. Please double check.\nShould be between 0 and 6.\n";
    inFile.close();
    //Everything should be set just as if user was entering info into the console
}

void saveFile()
{
    ofstream outFile;
    string Bz = "Bz", Bs = "Bs";
    outFile.open(fileName.c_str(), ios::trunc);
    outFile << RES << endl;
    outFile << totalPoly << endl;
    for (int step = 0; step < totalPoly; step++)
    {

    }
    outFile.close();
}

void getFileName()
{
    cout << "Please enter file name. Enter d for default name (POLYS.dat): \n";
    cin >> fileName;
    if (fileName.compare("d") == 0 || fileName.compare("D") == 0)
        fileName = "POLYS.dat";
    cout << "Loaded file: " << fileName << endl;

}

//Normalization functions
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
    THPolygon curPoly;
    for (int pStep = 0; pStep < totalPoly; pStep++)
    {
        curPoly = Polys[pStep];
        for (int step = 0; step < curPoly.totalVerts; step++)
        {
            x = curPoly.vertices[step].x;
            y = curPoly.vertices[step].y;

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

//Utility functions
void createEdges(THPolygon &poly)
{
    int pntPos = 0;
    poly.edges.clear();
    poly.totalEdges = 0;
    for (vector<TPnt>::iterator IT = poly.vertices.begin(); IT != poly.vertices.end() - 1; IT += 1)
    {
        poly.createEdge((*IT), (*(IT + 1)), pntPos, pntPos + 1);
        pntPos += 1;
    }
}

void displayPhong()
{
    //void phong(THPolygon &curPoly, float I_amb, float I_source, int nPhong, TPnt x, TPnt f, triple<float> k_amb, triple<float> k_dif, triple<float> k_spec)
    //cout << "Getting new Phong Input.\n";
    //cout << "I_Ambient: "; cin >> I_amb;
    //cout << "I_LightSource: "; cin >> I_source;
    //cout << "Phong Const: "; cin >> nPhong;
    //cout << "Light Position (x, y, z):\n";
    //cout << "x: "; cin >> lightPos.x;
    //cout << "y: "; cin >> lightPos.y;
    //cout << "z: "; cin >> lightPos.z;
    //cout << "Viewing Position (x, y, z):\n";
    //cout << "x: "; cin >> viewer.x;
    //cout << "y: "; cin >> viewer.y;
    //cout << "z: "; cin >> viewer.z;
    //cout << "k_amb, R: "; cin >> k_amb.i;
    //cout << "k_amb, G: "; cin >> k_amb.j;
    //cout << "k_amb, B: "; cin >> k_amb.k;
    //cout << "k_diff, R: "; cin >> k_diff.i;
    //cout << "k_diff, G: "; cin >> k_diff.j;
    //cout << "k_diff, B: "; cin >> k_diff.k;
    //cout << "k_spec, R: "; cin >> k_spec.i;
    //cout << "k_spec, G: "; cin >> k_spec.j;
    //cout << "k_spec, B: "; cin >> k_spec.k;
    //drawFaces();
    //raster();
}



void displayPolyInfo(int PolyID)
{
    THPolygon curPoly = Polys[PolyID];

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

/*
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
    PIXEL_BUFFER[step] = 0.0f;

    for (int pStep = 0; pStep < totalPoly; pStep++)   //For Each Poly and For each edge IN that poly
    {
        //Work with 1 polygon at a time. Sort the faces by z depth
        //sort faces by z depth
        vector<Face> sortedFaces;
        THPolygon curPoly = Polys[pStep];
        for (int i = 0; i < curPoly.totalFaces; i++)
            sortedFaces.push_back(curPoly.faces[i]);
        sort(sortedFaces.begin(), sortedFaces.end(), mySort);
        std::cout << "Sorted faces: \n";
        for (int i = 0; i < curPoly.totalFaces; i++)
            std::cout << i << ": " << sortedFaces[i].Pnt1.z << " | " << sortedFaces[i].Pnt2.z << " | " << sortedFaces[i].Pnt3.z << "\n";
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

    if (curPoly.edges[curEdge].slope == 0)  //Use x's
    {
        R = ((x - X1) / (float)(X2 - X1)) * I2.R + ((X2 - x) / (float)(X2 - X1)) * I1.R;
        B = ((x - X1) / (float)(X2 - X1)) * I2.G + ((X2 - x) / (float)(X2 - X1)) * I1.G;
        G = ((x - X1) / (float)(X2 - X1)) * I2.B + ((X2 - x) / (float)(X2 - X1)) * I1.B;
        //setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
    }
    else    //Use y's
    {
        R = ((x - Y1) / (float)(Y2 - Y1)) * I2.R + ((Y2 - x) / (float)(Y2 - Y1)) * I1.R;
        B = ((x - Y1) / (float)(Y2 - Y1)) * I2.G + ((Y2 - x) / (float)(Y2 - Y1)) * I1.G;
        G = ((x - Y1) / (float)(Y2 - Y1)) * I2.B + ((Y2 - x) / (float)(Y2 - Y1)) * I1.B;
        //setPixel(buffer, fabs(round(x)), fabs(round(y)), R, G, B);
    }
}

void raster()
{
    Rast = new float[WIDTH * HEIGHT * 3];

    int workingPoly = 0;
    float xy_x1, xy_y1, xy_x2, xy_y2;
    float xz_x1, xz_z1, xz_x2, xz_z2;
    float yz_y1, yz_z1, yz_y2, yz_z2;
    
    //Scan from y = 0 to y = MAX
    //Find all X intercepts for each edge ( see book )
    //From list of X intercepts, fill pairwise
    //List should be someting like    Y0 = (x1, x2, x3, x4)
    //Y1 = (x1, x2)
    //Would then fill from x1 - x2, x3 - x4 for Y0 then x1 - x2 for Y1
    
    //For each Poly
    //For each face
    //draw edges using interpolation in book
    //scan line between edges and interpolate RGB

    //No more -1 checking, only non-1 checking to find the x-intercept

    //Setup Poly Matrix
    for (workingPoly = 0; workingPoly < totalPoly; workingPoly++)  //Go through Poly's 1 by one
    {
        for (int i = 0; i < WIDTH * HEIGHT * 3; i++)
            Rast[i] = 1.0f;  //Set values for initial temp


        for (int curEdge = 0; curEdge < Polys[workingPoly].totalEdges; curEdge++) //Setup the poly in our blank matrix with -1's
        {
            xy_x1 = ((Polys[workingPoly].edges[curEdge].first.x - MastMin) / (float)(MastMax - MastMin)) * (WIDTH / 2);
            xy_y1 = ((Polys[workingPoly].edges[curEdge].first.y - MastMin) / (float)(MastMax - MastMin)) * ((HEIGHT - 1) / 2);


            xy_x2 = ((Polys[workingPoly].edges[curEdge].second.x - MastMin) / (float)(MastMax - MastMin)) * (WIDTH / 2);
            xy_y2 = ((Polys[workingPoly].edges[curEdge].second.y - MastMin) / (float)(MastMax - MastMin)) * ((HEIGHT - 1) / 2);

            rasterLines(xy_x1, xy_y1, xy_x2, xy_y2, R, G, B); //Now window is blank with poly edge's labeled as -1's
                                                              //rasterLines(xzBuf, xz_x1, xz_z1, xz_x2, xz_z2, R, G, B);
                                                              //rasterLines(yzBuf, yz_y1, yz_z1, yz_y2, yz_z2, R, G, B);
        }

        //Now scan each line. Where we encounter a -1, add to the X intercept vector
        Raster(Polys[workingPoly]);
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

void rasterLines(float X0, float Y0, float X_End, float Y_End, int R, int G, int B)
{
    float delta_X = X_End - X0, delta_Y = Y_End - Y0, steps; //delta's for X and Y and the number of steps we take in the for loop depending on the 0 < m < 1 stuff
    float xInc, yInc, Xn = X0, Yn = Y0; //Inc is the increment in the equations for Xn+1 and Yn+1 (either 1 or +/- m)

    if (delta_Y == 0) //Slope is undefined ---- vertical line
    {
        steps = fabs(delta_X);  //Steps toi move up the line
        xInc = float(delta_X) / float(steps);   //Increasing value for X
        rastPixel(round(Xn), round(Yn), R, G, B); //Set init point
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc;
            rastPixel(fabs(round(Xn)), fabs(round(Yn)), R, G, B);
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

        rastPixel(round(Xn), round(Yn), R, G, B);//Creates initial pixel
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc; //Increment X and Y by their respective values
            Yn += yInc;
            rastPixel(fabs(round(Xn)), fabs(round(Yn)), R, G, B);
        }
    }
}

void rastPixel(float x, float y, int R, int G, int B)
{   //Backround is Init to 1.0f = white, set pixel RGB to 0.0f for black

    //Copy paste of setPixel but for raster for the tempPoly

    for (int step = 0; step < 3; step++)
    {
        if (step == 0)
            Rast[(((int)x) * 3 + ((int)y) * WIDTH * 3) + step] = -1;  //Set R
        else if (step == 1)
            Rast[(((int)x) * 3 + ((int)y) * WIDTH * 3) + step] = -1;  //Set G
        else if (step == 2)
            Rast[(((int)x) * 3 + ((int)y) * WIDTH * 3) + step] = -1;  //Set B
    }


}

void Raster(THPolygon curPoly)
{
    vector<int> *xInterc = new vector<int>[HEIGHT];
    triple<float> I1(0, 0, 0);
    triple<float> I2(0, 0, 0);

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            if (Rast[x * 3 + y * WIDTH * 3] == -1)
            {
                xInterc[y].push_back(x);
            }
        }

        if (xInterc[y].size() != 0)
        {
            for (int bStep = 0; bStep < 3; bStep++)
            {
                if (bStep == 0)
                    I1.i = PIXEL_BUFFER[(((int)xInterc[y][0]) * 3 + ((int)y) * WIDTH * 3) + bStep];  //Set R
                else if (bStep == 1)
                    I1.j = PIXEL_BUFFER[(((int)xInterc[y][0]) * 3 + ((int)y) * WIDTH * 3) + bStep];  //Set G
                else if (bStep == 2)
                    I1.k = PIXEL_BUFFER[(((int)xInterc[y][0]) * 3 + ((int)y) * WIDTH * 3) + bStep];  //Set B
            }
            for (int bStep = 0; bStep < 3; bStep++)
            {
                if (bStep == 0)
                    I2.i = PIXEL_BUFFER[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * WIDTH * 3) + bStep];  //Set R
                else if (bStep == 1)
                    I2.j = PIXEL_BUFFER[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * WIDTH * 3) + bStep];  //Set G
                else if (bStep == 2)
                    I2.k = PIXEL_BUFFER[(((int)xInterc[y][xInterc[y].size() - 1]) * 3 + ((int)y) * WIDTH * 3) + bStep];  //Set B
            }
            fillDDA(xInterc[y][0], y, xInterc[y][xInterc[y].size() - 1], y, curPoly, I1, I2, 1.0f, 0.0f, 0.0f);
        }

    }
}

void fillDDA(float X0, int Y0, int X_End, int Y_End, THPolygon curPoly, triple<float> I1, triple<float> I2, float R, float G, float B)
{

    float delta_X = X_End - X0, delta_Y = Y_End - Y0, steps; //delta's for X and Y and the number of steps we take in the for loop depending on the 0 < m < 1 stuff
    float xInc, yInc, Xn = X0, Yn = Y0; //Inc is the increment in the equations for Xn+1 and Yn+1 (either 1 or +/- m)

    if (delta_Y == 0) //Slope is undefined ---- vertical line
    {
        steps = fabs(delta_X);  //Steps toi move up the line
        xInc = float(delta_X) / float(steps);   //Increasing value for X
        scanIntens(round(Xn), round(Yn), curPoly, I1, I2, X0, X_End);
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc;
            scanIntens(round(Xn), round(Yn), curPoly, I1, I2, X0, X_End);
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

        scanIntens(round(Xn), round(Yn), curPoly, I1, I2, X0, X_End);
        for (int k = 0; k < steps; k++)
        {
            Xn += xInc; //Increment X and Y by their respective values
            Yn += yInc;
            scanIntens(round(Xn), round(Yn), curPoly, I1, I2, X0, X_End);
        }
    }
}
//Halftoning
void halftone()
{
    //Will try implementing in Pixel Buffer XY
    //void setPixel(int Buffer, float x, float y, float R, float G, float B)
    //Im really sorry, this is gonna be MESSY code

    //XY
    for (int outerX = 0; outerX < WIDTH; outerX += 3)
    {
        for (int outerY = 0; outerY < HEIGHT; outerY += 3)
        {
            triple<float> total(0, 0, 0);
            for (int x = 0; x < 3; x++)
            {
                for (int y = 0; y < 3; y++)
                {
                    total.i += PIXEL_BUFFER[(((int)x + outerX) * 3 + ((int)y + outerY) * WIDTH * 3) + 0];
                    total.j += PIXEL_BUFFER[(((int)x + outerX) * 3 + ((int)y + outerY) * WIDTH * 3) + 1];
                    total.k += PIXEL_BUFFER[(((int)x + outerX) * 3 + ((int)y + outerY) * WIDTH * 3) + 2];
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
                    setPixel(x + outerX, y + outerY, total.i, total.j, total.k);
                }
            }
        }
    }
}

void scanIntens(int x, int y, THPolygon curPoly, triple<float> I1, triple<float> I2, float X0, float X_End)
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

    R = ((x - X1) / (float)(X2 - X1)) * I2.i + ((X2 - x) / (float)(X2 - X1)) * I1.i;
    B = ((x - X1) / (float)(X2 - X1)) * I2.j + ((X2 - x) / (float)(X2 - X1)) * I1.j;
    G = ((x - X1) / (float)(X2 - X1)) * I2.k + ((X2 - x) / (float)(X2 - X1)) * I1.k;
    setPixel(abs(round(x)), fabs(round(y)), R, G, B);

}

*/












