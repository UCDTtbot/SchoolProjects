#include "3DPolygon.h"
#include <iostream>

THPolygon::THPolygon()
{
    totalVerts = 0;
    totalEdges = 0;
    ID = -1;
    MastMax = 0;
    MastMin = 0;
}

THPolygon::THPolygon(int ID_in)
{
    ID = ID_in;
    totalVerts = 0;
    totalEdges = 0;
    MastMax = 0;
    MastMin = 0;
}

THPolygon::THPolygon(int ID_in, float xIn, float yIn, float zIn)
{
    ID = ID_in;
    totalEdges = 0;
    totalVerts = 0;
    addVert(xIn, yIn, zIn);
    MastMax = 0;
    MastMin = 0;
}

void THPolygon::addVert(float xIn, float yIn, float zIn)
{
    TPnt newPnt(xIn, yIn, zIn);
    vertices.push_back(newPnt);
    totalVerts++;
}

void THPolygon::setVert(int ID, float xIn, float yIn, float zIn)
{
    vertices[ID].x = xIn;
    vertices[ID].y = yIn;
    vertices[ID].z = zIn;

//    updateEdges();

}

void THPolygon::getCentroid()
{
    int totalX = 0, totalY = 0, totalZ = 0;
    //centroid is calculated by taking the avg of all vertices
    for (int step = 0; step < totalVerts; step++)
    {
        totalX += vertices[step].x;
        totalY += vertices[step].y;
        totalZ += vertices[step].z;
    }
    totalX /= totalVerts;
    totalY /= totalVerts;
    totalZ /= totalVerts;
    centroid.x = totalX; centroid.y = totalY; centroid.z = totalZ;
}

void THPolygon::createEdge(TPnt pnt1, TPnt pnt2, int c1, int c2)
{
    Edge newEdge(pnt1, pnt2, c1, c2);
    edges.push_back(newEdge);
    totalEdges++;
}

void THPolygon::updateEdges()
{
    
}

THPolygon::~THPolygon()
{
}
