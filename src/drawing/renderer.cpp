#include "drawing/renderer.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

#include "esp_dsp.h"

void RenderEngine::randomizeIt(){
    for (int i=0;i<numTriangles;i++){
        Vec2f v1(rand()%64,rand()%32);
        Vec2f v2(rand()%64,rand()%32);
        Vec2f v3(rand()%64,rand()%32);
        triangles[i] = Trianglef(v1,v2,v3, Devices::Display->color565(rand()%255,rand()%255,rand()%255)) ;
    }

    triangles[0] = Trianglef(Vec2f(8,16),Vec2f(16,16),Vec2f(16,8), Devices::Display->color565(255,0,0)) ;
    triangles[1] = Trianglef(Vec2f(8,16),Vec2f(16,8),Vec2f(8,8), Devices::Display->color565(0,255,0));
    triangles[2] = Trianglef(Vec2f(8,8),Vec2f(0,8),Vec2f(8,16), Devices::Display->color565(0,0,255));
    triangles[3] = Trianglef(Vec2f(16,8),Vec2f(24,8),Vec2f(16,16), Devices::Display->color565(100,100,100));

}

void RenderEngine::beginTriangles(){
    numTriangles = 1000;

    triangles = (Trianglef*)malloc(sizeof(Trianglef) * numTriangles);
    

    Serial.printf("done it\n");
    randomizeIt();

    started = true;
}
/*
Vec2f rotatePoint(const Vec2f& point, const Vec2f& center, float angle) {
    // Translate point to origin (relative to center)
    float translatedX = point.x - center.x;
    float translatedY = point.y - center.y;
    
    // Apply rotation
    float rotatedX = translatedX * cos(angle) - translatedY * sin(angle);
    float rotatedY = translatedX * sin(angle) + translatedY * cos(angle);
    
    // Translate back
    return Vec2f(rotatedX + center.x, rotatedY + center.y);
}

// Function to rotate a triangle around a center point
Trianglef rotateTriangle(const Trianglef& triangle, const Vec2f& center, float angle) {
    // Create rotated vertices
    Vec2f p1 = rotatePoint(Vec2f(triangle.p1X, triangle.p1Y), center, angle);
    Vec2f p2 = rotatePoint(Vec2f(triangle.p2X, triangle.p2Y), center, angle);
    Vec2f p3 = rotatePoint(Vec2f(triangle.p3X, triangle.p3Y), center, angle);
    
    // Return new rotated triangle with same color
    return Trianglef(p1, p2, p3, triangle.color);
}
*/

void RenderEngine::RenderTriangles() {
    if (!started){
        beginTriangles();
    }
    //Serial.printf("aaaa\n");

  
    uint8_t r,g,b;
    


    /*Vec2f rotationCenter(16, 8);

    float rotationAngle = 1 * M_PI / 180.0f;

    triangles[0] = rotateTriangle(triangles[0], rotationCenter, rotationAngle);
    triangles[1] = rotateTriangle(triangles[1], rotationCenter, rotationAngle);
    triangles[2] = rotateTriangle(triangles[2], rotationCenter, rotationAngle);
    triangles[3] = rotateTriangle(triangles[3], rotationCenter, rotationAngle);

    for (int i = 0; i < 4; i++) {
        triangles[i].recalculate();
    }*/

    uint32_t start = millis();
    Devices::Display->startWrite();
    for (int16_t y=0;y<PANEL_HEIGHT;y++){
      for (int16_t x=0;x<PANEL_WIDTH;x++){  
            int16_t color = Raster(triangles, numTriangles, Vec2f(x,y)); 
            Devices::Display->color565to888(color, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2(x, y, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH)+x, y, r, g, b);
      }
    }
    
    uint32_t diff = millis()-start;
    Devices::Display->endWrite();
    Serial.printf("Finished in %d ms\n", diff);
}

uint16_t RenderEngine::Raster(Trianglef* triangles, int numTriangles, Vec2f pixelRay) {
    //float zBuffer = 3.402823466e+38f;
    float u = 0.0f, v = 0.0f, w = 0.0f;
    //Vector3D uvw;
    for (int t = 0; t < numTriangles; t++) {
        //if (triangles[t].averageDepth < zBuffer) {
            if (triangles[t].DidIntersect(pixelRay.x, pixelRay.y, u, v, w)) {
                //uvw.X = u;
                //uvw.Y = v;
                //uvw.Z = w;
                //zBuffer = triangles[t].averageDepth;
                return triangles[t].color;
            }
        //}
    }

    return 0;
}
