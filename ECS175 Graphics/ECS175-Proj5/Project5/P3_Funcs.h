#pragma once
#include <GL/glut.h>
#include <stdio.h>
#include <iostream>
#include <utility>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <string.h>
#include <sstream>
#include <vector>
#include "triple.h"
#include "3DPolygon.h"
#include <map>

#ifndef P3_FUNCS_H
#define P3_FUNCS_H

//      Project 
//Phong Model Lighting
void phong(THPolygon &curPoly, float I_amb, float I_source, int nPhong, TPnt x, TPnt f, triple<float> k_amb, triple<float> k_dif, triple<float> k_spec);

//Gouraud Shading
void gouraud();

//Painter's Algorithm


//Halftone


#endif //P3_FUNCS_H