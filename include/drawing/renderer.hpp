#pragma once
#include <stdint.h>

#include "drawing/mathstuff/shapes.hpp"



class RenderEngine{
    public:
        RenderEngine():triangles(nullptr),numTriangles(0),started(false){};
        uint16_t Raster(Trianglef* triangles, int numTriangles, Vec2f pixelRay);
        uint16_t RasterVec(Trianglef* triangles, int numTriangles, Vec2f pixelRay);
        void RenderTriangles();

        Trianglef *triangles;
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
