#pragma once
#include <stdint.h>

class Vector3D {
public:
    float X;
    float Y;
    float Z;
};

class Vector2D {
public:
    Vector2D():X(0.0f),Y(0.0f){}
    Vector2D(float ax, float ay):X(ax),Y(ay){};
    float X;
    float Y;
};


class Triangle2D {
public:
    Triangle2D(){};
    Triangle2D(const Vector2D& p1, const Vector2D& p2, const Vector2D& p3, uint16_t color);

    float denominator = 0.0f; ///< Precomputed denominator for barycentric coordinate calculations.
    float p1X, p1Y, p2X, p2Y, p3X, p3Y; ///< Coordinates of the triangle's vertices.
    float v0X, v0Y, v1X, v1Y, v2X, v2Y; ///< Edge vectors for barycentric coordinate calculations.
    Vector2D min, max; ///< Minimum and maximum bounds of the triangle.

    Vector3D* normal;
    //Material* material; ///< Material assigned to the triangle.

    Vector3D* t3p1; ///< Pointer to the first vertex in 3D space.
    Vector3D* t3p2; ///< Pointer to the second vertex in 3D space.
    Vector3D* t3p3; ///< Pointer to the third vertex in 3D space.

    const Vector2D* p1UV; ///< UV coordinates of the first vertex.
    const Vector2D* p2UV; ///< UV coordinates of the second vertex.
    const Vector2D* p3UV; ///< UV coordinates of the third vertex.

    bool hasUV = false; ///< Indicates whether the triangle has UV mapping.
    float averageDepth = 0.0f; ///< Average depth of the triangle for rendering.

    uint16_t color;

    //Triangle2D(Triangle3D* t);

    bool DidIntersect(const float& x, const float& y, float& u, float& v, float& w);

};

class RenderEngine{
    public:
        RenderEngine():triangles(nullptr),numTriangles(0),started(false){};
        uint16_t Raster(Triangle2D* triangles, int numTriangles, Vector2D pixelRay);
        uint16_t RasterVec(Triangle2D* triangles, int numTriangles, Vector2D pixelRay);
        void RenderTriangles();

        Triangle2D *triangles;
        int numTriangles;
        bool started;
        void randomizeIt();

        void beginTriangles();


        float* subA;
        float* subB;
        float* subDst;

        float* multA;
        float* multB;
        float* multDst;

        float* sub1A;
        float* sub1B;
        float* sub1Dst;

        float* mult1A;
        float* mult1B;
        float* mult1Dst;


};
