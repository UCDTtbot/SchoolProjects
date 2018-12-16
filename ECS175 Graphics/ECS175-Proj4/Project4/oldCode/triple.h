#pragma once
#ifndef _TRIPLE_H
#define _TRIPLE_H
template <class T>
class triple
{

public:
    
    T i, j, k;    //Normal vector (cross product)

    triple() {};
    triple(T fir, T sec, T thir) { i = fir; j = sec; k = thir; };
    ~triple() {};

};
#endif //_TRIPLE_H
