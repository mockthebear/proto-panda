#include "drawing/renderer.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

static Modelf mem;

void RenderEngine::randomizeIt(){
    

}

void RenderEngine::beginTriangles(){
    numTriangles = 3;

    Serial.printf("done it\n");
    mem.begin(numTriangles);
    
mem.setTriangle(0, Trianglef(Vec2f(0.0f,0.0f), Vec2f(63.0f,31.0f), Vec2f(0.0f,31.0f), Devices::Display->color565(255,0,0)));
mem.setTriangle(1, Trianglef(Vec2f(6.0f,0.0f), Vec2f(63.0f,30.0f), Vec2f(63.0f,0.0f), Devices::Display->color565(0,255,0)));

mem.recalculate();
    Serial.printf("done it\n");

    started = true;
}


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
    //mem.rotate(Vec2f(32,16), 1 * M_PI / 180.0f);
    uint32_t diffOp = millis()-start;
    start = millis();
    //Devices::Display->clearScreen();
    Devices::Display->startWrite();
    float u = 0.0f, v = 0.0f, w = 0.0f;
    for (float y=0;y<PANEL_HEIGHT;y++){
      for (float x=0;x<PANEL_WIDTH;x++){  
            uint16_t color = 0;
            if (!mem.DidIntersect(x, y, u, v, w, color)){
                color = 0;
            }
            //Raster(triangles, numTriangles, Vec2f(x,y)); 
            Devices::Display->color565to888(color, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2(x, y, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH)+x, y, r, g, b);
      }
    }
    
    uint32_t diff = millis()-start;
    Devices::Display->endWrite();
    Serial.printf("Transformed in %d and rendered in %d ms (%2.2f fps)\n", diffOp, diff, 1000.0f/(float)diff);
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
