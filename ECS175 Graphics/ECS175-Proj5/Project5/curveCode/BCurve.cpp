#include "BCurve.h"
#include <iostream>

BCurve::BCurve()
{
    totalPoints = 0;
    totalEdges = 0;
    TYPE = "NULL";
    ID = -1;
}

BCurve::BCurve(int ID_in, string type)
{
    ID = ID_in;
    TYPE = type;
    totalPoints = 0;
    totalEdges = 0;
}

BCurve::BCurve(int ID_in, string type, float xIn, float yIn)
{
    ID = ID_in;
    TYPE = type;
    totalPoints = 0;
    totalEdges = 0;
    addPoints(xIn, yIn);
}

void BCurve::addPoints(float xIn, float yIn)
{
    Pnt newPnt(xIn, yIn);
    controlPoints.push_back(newPnt);
    totalPoints++;
}

void BCurve::removePoint(int pos)
{
    controlPoints.erase(controlPoints.begin()+pos);
    totalPoints--;
}

void BCurve::setPoint(int pos, float xIn, float yIn)
{
    controlPoints[pos].x = xIn;
    controlPoints[pos].y = yIn;
}

void BCurve::addEdge(Pnt pnt1, Pnt pnt2, int c1, int c2)
{
    Edge newEdge(pnt1, pnt2, c1, c2);
    controlEdges.push_back(newEdge);
    totalEdges++;
}

void BCurve::removeEdge(int c1, int c2)
{
    int offset = 0;
    for (vector<Edge>::iterator IT = controlEdges.begin(); IT != controlEdges.end(); IT++)
    {
        if ((*IT).fstCoord == c1 && (*IT).scdCoord == c2)
            controlEdges.erase(IT);
        offset++;
    }
}

BCurve::~BCurve()
{

}