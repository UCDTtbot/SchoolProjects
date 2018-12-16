#pragma once

#ifndef PNT_H
#define PNT_H
class Pnt
{
public:
    float x, y;

    Pnt() { x = 0; y = 0; };
    Pnt(float inX, float inY) { x = inX; y = inY; }
    ~Pnt() {};
};
#endif