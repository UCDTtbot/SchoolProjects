#pragma once
#include "2DPoint.h"
#include "triple.h"

#ifndef TPNT_H
#define TPNT_H

class TPnt : public Pnt
{
public:
    float z;
    float R, G, B;
    triple<float> vertexNormal;
    //Vertex normal is found by normalizing the three adjacent faces
    TPnt() { x = 0; y = 0; z = 0; };
    TPnt(float xIn, float yIn, float zIn) { x = xIn; y = yIn; z = zIn; };
    TPnt(float xIn, float yIn, float zIn, int r, int g, int b)
    {   x = xIn; y = yIn; z = zIn; R = r; G = g; B = b; };
    ~TPnt() {};

};


#endif //3DPNT_H