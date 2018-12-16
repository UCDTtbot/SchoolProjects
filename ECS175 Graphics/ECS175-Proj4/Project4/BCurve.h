#pragma once
#include <utility>
#include <vector>
#include <string>
#include "2DPoint.h"
#include "Edge.h"

#ifndef BCURVE_H
#define BCURVE_H
using namespace std;

class BCurve
{
public:
    vector<Pnt> controlPoints;
    vector<Edge> controlEdges;
    vector<int> knots;
    string TYPE;
    bool knotBool;
    int kValue;
    int numKnots;
    int totalPoints;
    int totalEdges;
    int ID;

    BCurve();
    BCurve(int ID, string);
    BCurve(int ID, string, float x, float y);
    void addPoints(float x, float y);
    void addEdge(Pnt, Pnt, int, int);
    void removeEdge(int, int);
    void removePoint(int pos);
    void setPoint(int pos, float x, float y);

    ~BCurve();
};



#endif //BCURVE