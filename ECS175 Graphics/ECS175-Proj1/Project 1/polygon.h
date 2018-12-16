#pragma once
#include <utility>
#include <vector>
#include "2DPoint.h"
#include "Edge.h"

#ifndef POLYGON_H
#define POLYGON_H
using namespace std;

class polygon
{
private:
    vector<Pnt> vertices;   //vertice pair
    vector<Edge> edges;
    int totalVerts; //Total verts in use | Use for indexing the next vert to add
    int totalEdges;
    Pnt centroid;
    int ID;         //ID of polygon


public:
    polygon();          //default
    polygon(int ID);    //Only ID given
    polygon(int ID, int x, int y);
    void addVert(int x, int y);
    void setVert(int ID, int x, int y);
    void clearEdges() { edges.clear(); totalEdges = 0; }
    void populateEdges();
    void getCentroid();
    void listEdges();
    int getID() { return ID; }
    int getVertSize();
    Pnt getVert(int ID);
    Pnt getCenter() { return centroid; }
    ~polygon();
};
#endif

