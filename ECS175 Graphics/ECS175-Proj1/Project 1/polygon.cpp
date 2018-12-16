#include "polygon.h"
#include <iostream>

polygon::polygon()
{
    totalVerts = 0;
    ID = -1;
}

polygon::polygon(int ID_in)
{
    ID = ID_in;
    totalVerts = 0;
}

polygon::polygon(int ID_in, int xIn, int yIn)
{
    ID = ID_in;
    addVert(xIn, yIn);
}

void polygon::addVert(int x, int y)
{
    Pnt newPnt(x, y);
    vertices.push_back(newPnt);
    totalVerts++;
}

Pnt polygon::getVert(int ID)
{
    return vertices[ID];
}

void polygon::setVert(int ID, int x, int y)
{
    vertices[ID].setX(x);
    vertices[ID].setY(y);
}

void polygon::populateEdges()
{
    int firstV = 0;
    for (int step = 0; step < totalVerts - 1; step++) // for first vert to second to last
    {
        Edge newEdge(vertices[step], vertices[step + 1]);
        edges.push_back(newEdge);
        totalEdges++;
    }
    Edge newEdge(vertices[totalVerts - 1], vertices[firstV]);
    edges.push_back(newEdge);
    totalEdges++;
}

void polygon::getCentroid()
{
    int totalX = 0, totalY = 0;
    //centroid is calculated by taking the avg of all vertices
    for (int step = 0; step < totalVerts; step++)
    {
        totalX += vertices[step].getX();
        totalY += vertices[step].getY();
    }
    totalX /= totalVerts;
    totalY /= totalVerts;
    centroid.setPnts(totalX, totalY);
}

int polygon::getVertSize()
{
    return totalVerts;
}

void polygon::listEdges()
{
    std::cout << "Edges for Polygon #" << ID + 1 << ":\n";
    for (int step = 0; step < totalEdges; step++)
    {
        cout << "Edge #" << step << " (" << edges[step].getFirst().getX() << ", " << edges[step].getFirst().getY() << ") | (" << edges[step].getSecond().getX() << ", " << edges[step].getSecond().getY() << ")\n";
    }
}

polygon::~polygon()
{
}
