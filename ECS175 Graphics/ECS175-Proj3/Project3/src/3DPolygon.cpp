#include "../include/3DPolygon.h"
#include <iostream>

THPolygon::THPolygon()
{
    totalVerts = 0;
    totalEdges = 0;
    totalFaces = 0;
    ID = -1;
}

THPolygon::THPolygon(int ID_in)
{
    ID = ID_in;
    totalVerts = 0;
    totalEdges = 0;
    totalFaces = 0;
}

THPolygon::THPolygon(int ID_in, float xIn, float yIn, float zIn)
{
    ID = ID_in;
    totalEdges = 0;
    totalVerts = 0;
    totalFaces = 0;
    addVert(xIn, yIn, zIn);
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

void THPolygon::createFace(TPnt pnt1, TPnt pnt2, TPnt pnt3, int p1, int p2, int p3)
{
    Face newFace(pnt1, pnt2, pnt3, p1, p2, p3);
    newFace.createEdge(pnt1, pnt2, p1, p2);
    newFace.createEdge(pnt2, pnt3, p2, p3);
    newFace.createEdge(pnt3, pnt1, p3, p1);
    faces.push_back(newFace);
    totalFaces++;
}

void THPolygon::updateEdges()
{
    
}

THPolygon::~THPolygon()
{
}
