#pragma once
#include "2DPoint.h"

#ifndef TPNT_H
#define TPNT_H

class TPnt : public Pnt
{
public:
    float z;

    TPnt() { x = 0; y = 0; z = 0; };
    TPnt(float xIn, float yIn, float zIn) { x = xIn; y = yIn; z = zIn; };
    ~TPnt() {};

};


#endif //3DPNT_H