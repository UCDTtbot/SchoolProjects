#pragma once
#include "2DPoint.h"

#ifndef EDGE_H
#define EDGE_H
class Edge
{
private:
    Pnt first;
    Pnt second;
    float slope;


public:
    Edge() { first.setPnts(0, 0); second.setPnts(0, 0); }
    Edge(Pnt inFirst, Pnt inSecond) { 
        first = inFirst; second = inSecond;
        if ((second.getX() - first.getX()) != 0)
            slope = (second.getY() - first.getY()) / (second.getX() - first.getX());
        else
            slope = 0;
        }
    float getSlope() { return slope; }
    Pnt getFirst() { return first; }
    Pnt getSecond() { return second; }
    ~Edge() {};

};
#endif