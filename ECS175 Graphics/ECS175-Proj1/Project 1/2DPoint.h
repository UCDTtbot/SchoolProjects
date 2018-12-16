#pragma once
#include "Window.h"

#ifndef PNT_H
#define PNT_H
//int WIDTH, HEIGHT, MID_X, MID_Y; //EXTERNED VARIABLES FROM WINDOW.H
class Pnt
{
private:
    float x, y;
public:
    Pnt() { x = 0; y = 0; };
    Pnt(float inX, float inY) { x = inX; y = inY; }
    ~Pnt() {};
    void setPnts(float inX, float inY) { x = inX; y = inY; }
    void setX(float inX) { x = inX; }// +MID_X;
    void setY(float inY) { y = inY; }// +MID_Y;
    float getX() { return x; }// -MID_X; }
    float getY() { return y; }// -MID_Y; }

};
#endif