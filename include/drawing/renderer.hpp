#pragma once
#include <stdint.h>

#include "drawing/mathstuff/shapes.hpp"



class RenderEngine{
    public:
        RenderEngine():numTriangles(0),started(false){};
        uint16_t Raster(Trianglef* triangles, int numTriangles, Vec2f pixelRay);
        void RenderTriangles();

      
        int numTriangles;
        bool started;
        void randomizeIt();

        void beginTriangles();




};
