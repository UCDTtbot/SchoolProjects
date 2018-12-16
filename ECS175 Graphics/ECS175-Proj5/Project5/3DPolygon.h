#pragma once
#include <utility>
#include <vector>
#include "3DPoint.h"
#include "Edge.h"
#include "face.h"

#ifndef THPOLYGON_H
#define THPOLYGON_H
using namespace std;

class THPolygon
{
public:
    //Store all verts as unit 
    vector<TPnt> vertices;      //vertice pair  - WORLD COORDS
    vector<Edge> edges;         //edges         - WORLD COORDS
    vector<Face> faces;
    int totalVerts; //Total verts in use | Use for indexing the next vert to add
    int totalEdges;
    int totalFaces;
    TPnt centroid;
    int ID;         //ID of polygon

    THPolygon();          //default
    THPolygon(int ID);    //Only ID given
    THPolygon(int ID, float x, float y, float z);
    void addVert(float x, float y, float z);
    void setVert(int ID, float x, float y, float z);
    void getCentroid();
    void createEdge(TPnt pnt1, TPnt pnt2, int c1, int c2);
    void createFace(TPnt pnt1, TPnt pnt2, TPnt pnt3, int p1, int p2, int p3);
    void updateEdges();

    ~THPolygon();
};
#endif

