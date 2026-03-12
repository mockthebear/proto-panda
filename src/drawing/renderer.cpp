#include "drawing/renderer.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

void RenderEngine::randomizeIt(){
    

}

void RenderEngine::beginTriangles(){
    

    Serial.printf("done it\n");
    Model *mem = new Model();
    numTriangles = 24;
    mem->Begin(numTriangles);
    
    mem->SetTriangle(0, Vec2f(5.0f,19.0f), Vec2f(8.0f,22.0f), Vec2f(12.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(1, Trianglef(Vec2f(5.0f,19.0f), Vec2f(8.0f,18.0f), Vec2f(12.0f,20.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(2, Trianglef(Vec2f(7.0f,18.0f), Vec2f(9.0f,18.0f), Vec2f(8.0f,20.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(3, Trianglef(Vec2f(11.0f,21.0f), Vec2f(18.0f,27.0f), Vec2f(21.0f,27.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(4, Trianglef(Vec2f(11.0f,21.0f), Vec2f(14.0f,21.0f), Vec2f(21.0f,27.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(5, Trianglef(Vec2f(18.0f,27.0f), Vec2f(26.0f,20.0f), Vec2f(28.0f,20.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(6, Trianglef(Vec2f(18.0f,27.0f), Vec2f(21.0f,27.0f), Vec2f(28.0f,20.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(7, Trianglef(Vec2f(26.0f,20.0f), Vec2f(33.0f,27.0f), Vec2f(35.0f,27.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(8, Trianglef(Vec2f(26.0f,20.0f), Vec2f(28.0f,20.0f), Vec2f(35.0f,27.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(9, Trianglef(Vec2f(33.0f,27.0f), Vec2f(35.0f,27.0f), Vec2f(42.0f,20.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(10, Trianglef(Vec2f(42.0f,20.0f), Vec2f(40.0f,20.0f), Vec2f(33.0f,27.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(11, Trianglef(Vec2f(40.0f,20.0f), Vec2f(48.0f,27.0f), Vec2f(50.0f,27.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(12, Trianglef(Vec2f(40.0f,20.0f), Vec2f(42.0f,20.0f), Vec2f(50.0f,27.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(13, Trianglef(Vec2f(60.0f,5.0f), Vec2f(55.0f,5.0f), Vec2f(60.0f,8.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(14, Trianglef(Vec2f(7.0f,6.0f), Vec2f(11.0f,8.0f), Vec2f(14.0f,6.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(15, Trianglef(Vec2f(7.0f,6.0f), Vec2f(9.0f,5.0f), Vec2f(12.0f,5.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(16, Trianglef(Vec2f(12.0f,5.0f), Vec2f(14.0f,6.0f), Vec2f(16.0f,5.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(17, Trianglef(Vec2f(9.0f,5.0f), Vec2f(16.0f,5.0f), Vec2f(13.0f,3.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(18, Trianglef(Vec2f(14.0f,6.0f), Vec2f(16.0f,5.0f), Vec2f(17.0f,6.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(19, Trianglef(Vec2f(11.0f,8.0f), Vec2f(14.0f,6.0f), Vec2f(17.0f,8.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(20, Trianglef(Vec2f(17.0f,8.0f), Vec2f(17.0f,6.0f), Vec2f(14.0f,6.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(21, Trianglef(Vec2f(17.0f,6.0f), Vec2f(18.0f,7.0f), Vec2f(17.0f,8.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(22, Trianglef(Vec2f(12.0f,20.0f), Vec2f(11.0f,21.0f), Vec2f(9.0f,18.0f), Devices::Display->color565(0,0,255)));
    mem->SetTriangle(23, Trianglef(Vec2f(12.0f,20.0f), Vec2f(11.0f,21.0f), Vec2f(8.0f,20.0f), Devices::Display->color565(0,0,255)));
    mem->Recalculate();
    mem->SetBatchOperations(true);
    mem->SetAccumulativeOperations(true);

    mem->AddPointGroup(std::vector<int>({17, 16, 20, 25, 24,21})); //


    sc.addModel(mem);
    Serial.printf("done it\n");

    started = true;
}


void RenderEngine::RenderTriangles() {
    if (!started){
        beginTriangles();
    }
    //Serial.printf("aaaa\n");



    /*Vec2f rotationCenter(16, 8);

    float rotationAngle = 1 * M_PI / 180.0f;

    triangles[0] = rotateTriangle(triangles[0], rotationCenter, rotationAngle);
    triangles[1] = rotateTriangle(triangles[1], rotationCenter, rotationAngle);
    triangles[2] = rotateTriangle(triangles[2], rotationCenter, rotationAngle);
    triangles[3] = rotateTriangle(triangles[3], rotationCenter, rotationAngle);

    for (int i = 0; i < 4; i++) {
        triangles[i].recalculate();
    }*/
    static float scaled = 1.0f;
    static float increment = (8.0f * M_PI / 180.0f);  

    static float rotated = 0.0f;
    static float incrRotated = (0.5f * M_PI / 180.0f);
    rotated += incrRotated;

    static float rotated2 = 0.0f;
    static float incrRotated2 = (-2.0f * M_PI / 180.0f);
    rotated2 += incrRotated2;

    if (rotated > (15.0f * M_PI / 180.0f)){
        incrRotated = -(0.5f * M_PI / 180.0f);
        rotated = (15.0f * M_PI / 180.0f);
    }else if(rotated < (-15.0f * M_PI / 180.0f)){
        rotated = (-15.0f * M_PI / 180.0f);
        incrRotated = (0.5f * M_PI / 180.0f);
    }

    scaled += increment;

    //
    
    sc.models[0]->Reset();
    sc.models[0]->Rotate(Vec2f(PANEL_WIDTH/2.0f, PANEL_HEIGHT/2.0f), rotated);
    sc.models[0]->Scale(Vec2f(PANEL_WIDTH/2.0f, PANEL_HEIGHT/2.0f), Vec2f(-1.0f + 0.25f * sin(scaled), 1.0f + 0.15f * cos(scaled)));

    sc.models[0]->TranslatePoints(0, Vec2f(0.0f, 16.0f * cos(scaled)));
    sc.models[0]->Translate(Vec2f(10.0f * sin(rotated2), 10.0f * cos(rotated2)));
    sc.models[0]->Recalculate();

    uint32_t start = millis();
    sc.RenderScene();

    uint32_t diff = millis()-start;
    //start = millis();
    //Devices::Display->clearScreen();
    
    Serial.printf("Rendered in %d ms (%2.2f fps)\n", diff, 1000.0f/(float)diff);
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
