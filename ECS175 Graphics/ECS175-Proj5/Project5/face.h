#pragma once
#include "triple.h"
#include "3DPoint.h"
#include "Edge.h"

#ifndef _FACE_H
#define _FACE_H
using namespace std;

class Face
{

public:
    triple<float> normal;
    TPnt Pnt1, Pnt2, Pnt3;
    vector<Edge> edges;
    int totalEdges;
    int first, second, third;


    Face() {};
    Face(TPnt one, TPnt two, TPnt three, int fir, int sec, int thir) { Pnt1 = one; Pnt2 = two; Pnt3 = three; first = fir; second = sec; third = thir; };
    void createEdge(TPnt pnt1, TPnt pnt2, int c1, int c2)
    {
        Edge newEdge(pnt1, pnt2, c1, c2);
        edges.push_back(newEdge);
        totalEdges++;
    };
    ~Face() {};
};

#endif //_FACE_F
