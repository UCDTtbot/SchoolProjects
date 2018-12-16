#pragma once
#include "3DPoint.h"

#ifndef EDGE_H
#define EDGE_H
class Edge
{

public:
    TPnt first;
    TPnt second;
    float slope;
    int fstCoord, scdCoord;
    Edge() { first.x = 0; first.y = 0, first.z = 0; second.x = 0; second.y = 0; second.z = 0; }
    Edge(TPnt inFirst, TPnt inSecond, int fstC, int scdC) { 
        first = inFirst; second = inSecond;
        fstCoord = fstC; scdCoord = scdC;
        }
    ~Edge() {};

};
#endif