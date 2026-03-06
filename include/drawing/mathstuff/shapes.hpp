#pragma once 
#include "drawing/mathstuff/primitives.hpp"

#include <stdint.h>

template<typename T> class Triangle {
public:
    Triangle():denominator(static_cast<T>(0)){};
    Triangle(const Vec2<T>& p1, const Vec2<T>& p2, const Vec2<T>& p3, uint16_t color){
        p1X = p1.x;
        p1Y = p1.y;
        p2X = p2.x;
        p2Y = p2.y;
        p3X = p3.x;
        p3Y = p3.y;
        color = color;

        v0X = p2X - p1X;
        v0Y = p2Y - p1Y;
        v1X = p3X - p1X;
        v1Y = p3Y - p1Y;

        denominator = 1.0f / (v0X * v1Y - v1X * v0Y);
    }
    

    T denominator; 
    T p1X, p1Y, p2X, p2Y, p3X, p3Y; 
    T v0X, v0Y, v1X, v1Y; 

    uint16_t color;

    bool DidIntersect(const T& x, const T& y, T& u, T& v, T& w){
        T v2lX = x - p1X;
        T v2lY = y - p1Y;

        v = (v2lX * v1Y - v1X * v2lY) * denominator;
        w = (v0X * v2lY - v2lX * v0Y) * denominator;

        if (v < static_cast<T>(0.0) || w < static_cast<T>(0.0) || v > static_cast<T>(1.0) || w > static_cast<T>(1.0)) return false;

        u = static_cast<T>(1.0) - v - w;

        return u >= static_cast<T>(0.0f);
    }

};

template<typename T, int N> class Shape {
    public:
        const int getSize(){ return N;};
        Triangle<T> elements[M];
};

template<typename T> class Square : public Shape<2>  {
public:
    Square(){};
    Square(const Vec2<T>& c1, const Vec2<T>& c2, const Vec2<T>& c3, const Vec2<T>& c4, uint16_t color=0){
        elements[0] = Triangle<T>(c1, c2, c3, color);
        elements[1] = Triangle<T>(c3, c4, c1, color);
    }

    void setColor(uint16_t color){
        elements[0].color = elements[1].color = color;
    }
};



typedef Triangle<int> Trianglei;
typedef Triangle<float> Trianglef;