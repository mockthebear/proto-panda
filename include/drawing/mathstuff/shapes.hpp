#pragma once 
#include "drawing/mathstuff/primitives.hpp"

#include <stdint.h>

template<typename T> class Triangle {
public:
    Triangle(){};
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


    float denominator = 0.0f; ///< Precomputed denominator for barycentric coordinate calculations.
    float p1X, p1Y, p2X, p2Y, p3X, p3Y; ///< Coordinates of the triangle's vertices.
    float v0X, v0Y, v1X, v1Y, v2X, v2Y; ///< Edge vectors for barycentric coordinate calculations.
    Vec2<T> min, max; ///< Minimum and maximum bounds of the triangle.

    Vec3<T>* normal;
    //Material* material; ///< Material assigned to the triangle.

    Vec3<T>* t3p1; ///< Pointer to the first vertex in 3D space.
    Vec3<T>* t3p2; ///< Pointer to the second vertex in 3D space.
    Vec3<T>* t3p3; ///< Pointer to the third vertex in 3D space.

    const Vec2<T>* p1UV; ///< UV coordinates of the first vertex.
    const Vec2<T>* p2UV; ///< UV coordinates of the second vertex.
    const Vec2<T>* p3UV; ///< UV coordinates of the third vertex.

    bool hasUV = false; ///< Indicates whether the triangle has UV mapping.
    float averageDepth = 0.0f; ///< Average depth of the triangle for rendering.

    uint16_t color;

    //Triangle2D(Triangle3D* t);

    bool DidIntersect(const float& x, const float& y, float& u, float& v, float& w){
        float v2lX = x - p1X;
        float v2lY = y - p1Y;

        v = (v2lX * v1Y - v1X * v2lY) * denominator;
        w = (v0X * v2lY - v2lX * v0Y) * denominator;

        if (v < 0.0f || w < 0.0f || v > 1.0f || w > 1.0f) return false;

        u = 1.0f - v - w;

        return u >= 0.0f;
    }

};



typedef Triangle<int> Trianglei;
typedef Triangle<float> Trianglef;