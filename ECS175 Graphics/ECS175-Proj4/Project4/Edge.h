#pragma once
#include "2DPoint.h"

#ifndef EDGE_H
#define EDGE_H
class Edge
{

public:
    Pnt first;
    Pnt second;
    float slope;
    int fstCoord, scdCoord;
    Edge() { first.x = 0; first.y = 0; second.x = 0; second.y = 0; }
    Edge(Pnt inFirst, Pnt inSecond, int fstC, int scdC) { 
        first = inFirst; second = inSecond;
        fstCoord = fstC; scdCoord = scdC;
        }
    ~Edge() {};

};
#endif