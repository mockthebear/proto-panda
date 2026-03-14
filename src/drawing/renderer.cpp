#include "drawing/renderer.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

void RenderEngine::randomizeIt(){
    

}

void RenderEngine::beginTriangles(){
    

    Serial.printf("done it\n");
    Model *mem;
    
    // Model 0: "Model 0"
    // Model 0: "Model 0"
        // Model 0: "Model 0"
    mem = new Model();
    numTriangles = 15;
    mem->Begin(numTriangles);
    mem->SetTriangle(0, Vec2f(10.0f,21.0f), Vec2f(18.0f,27.0f), Vec2f(21.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(1, Vec2f(10.0f,21.0f), Vec2f(12.0f,20.0f), Vec2f(21.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(2, Vec2f(18.0f,27.0f), Vec2f(26.0f,20.0f), Vec2f(28.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(3, Vec2f(18.0f,27.0f), Vec2f(21.0f,27.0f), Vec2f(28.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(4, Vec2f(26.0f,20.0f), Vec2f(33.0f,27.0f), Vec2f(35.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(5, Vec2f(26.0f,20.0f), Vec2f(28.0f,20.0f), Vec2f(35.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(6, Vec2f(33.0f,27.0f), Vec2f(35.0f,27.0f), Vec2f(42.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(7, Vec2f(42.0f,20.0f), Vec2f(40.0f,20.0f), Vec2f(33.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(8, Vec2f(40.0f,20.0f), Vec2f(48.0f,27.0f), Vec2f(50.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(9, Vec2f(40.0f,20.0f), Vec2f(42.0f,20.0f), Vec2f(50.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(10, Vec2f(4.0f,18.0f), Vec2f(4.0f,20.0f), Vec2f(10.0f,21.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(11, Vec2f(4.0f,18.0f), Vec2f(12.0f,20.0f), Vec2f(10.0f,21.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(12, Vec2f(9.0f,16.0f), Vec2f(4.0f,18.0f), Vec2f(12.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(13, Vec2f(9.0f,16.0f), Vec2f(6.0f,16.0f), Vec2f(4.0f,18.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(14, Vec2f(4.0f,20.0f), Vec2f(10.0f,21.0f), Vec2f(6.0f,22.0f), Devices::Display->color565(0,0,255));

    mem->AddPointGroup(std::vector<int>({1, 2, 5, 6, 9, 10}));
    mem->AddPointGroup(std::vector<int>({7, 8, 11, 12, 15, 16}));
    mem->AddPointGroup(std::vector<int>({13, 14, 17, 18, 19, 23}));
    mem->AddPointGroup(std::vector<int>({20, 21, 22, 24, 27, 28}));
    mem->AddPointGroup(std::vector<int>({25, 26, 29}));
    mem->Reset();
    mem->Recalculate();
    mem->SetBatchOperations(true);
    mem->SetAccumulativeOperations(true);
    sc.addModel(mem);

    // Model 1: "Model 1"
    mem = new Model();
    numTriangles = 7;
    mem->Begin(numTriangles);
    mem->SetTriangle(0, Vec2f(7.0f,6.0f), Vec2f(14.0f,6.0f), Vec2f(11.0f,8.0f), Devices::Display->color565(102,102,255));
    mem->SetTriangle(1, Vec2f(9.0f,5.0f), Vec2f(14.0f,6.0f), Vec2f(7.0f,6.0f), Devices::Display->color565(102,102,255));
    mem->SetTriangle(2, Vec2f(13.0f,3.0f), Vec2f(9.0f,5.0f), Vec2f(14.0f,6.0f), Devices::Display->color565(102,102,255));
    mem->SetTriangle(3, Vec2f(13.0f,3.0f), Vec2f(16.0f,5.0f), Vec2f(14.0f,6.0f), Devices::Display->color565(102,102,255));
    mem->SetTriangle(4, Vec2f(14.0f,6.0f), Vec2f(11.0f,8.0f), Vec2f(17.0f,8.0f), Devices::Display->color565(102,102,255));
    mem->SetTriangle(5, Vec2f(14.0f,6.0f), Vec2f(18.0f,7.0f), Vec2f(17.0f,8.0f), Devices::Display->color565(102,102,255));
    mem->SetTriangle(6, Vec2f(16.0f,5.0f), Vec2f(14.0f,6.0f), Vec2f(18.0f,7.0f), Devices::Display->color565(102,102,255));

    mem->AddPointGroup(std::vector<int>({3, 6, 7, 9, 10, 18}));
    mem->AddPointGroup(std::vector<int>({2, 13, 14, 16, 17}));
    mem->Reset();
    mem->Recalculate();
    mem->SetBatchOperations(true);
    mem->SetAccumulativeOperations(true);
    sc.addModel(mem);

    // Model 2: "Model 2"
    mem = new Model();
    numTriangles = 1;
    mem->Begin(numTriangles);
    mem->SetTriangle(0, Vec2f(55.0f,5.0f), Vec2f(60.0f,5.0f), Vec2f(60.0f,8.0f), Devices::Display->color565(100,100,255));
    mem->Reset();
    mem->Recalculate();
    mem->SetBatchOperations(true);
    mem->SetAccumulativeOperations(true);
    sc.addModel(mem);

    
    sc.addModel(mem);





    Serial.printf("done it\n");

    started = true;
}


void RenderEngine::RenderTriangles() {
    if (!started){
        return;
        //beginTriangles();
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
    static float incrRotated = (2.0f * M_PI / 180.0f);
    rotated += incrRotated;

    static float rotated2 = 0.0f;
    static float incrRotated2 = (6.0f * M_PI / 180.0f);
    rotated2 += incrRotated2;


    scaled += increment;

    //
    uint32_t start = millis();
    sc.models[0]->Reset();
    sc.models[1]->Reset();
    sc.models[2]->Reset();
    
    //originalCenter

    sc.models[1]->ScalePoints(0,  Vec2f(13.0f, 7.0f), Vec2f(1.0f, fabsf(sin(rotated))));
    sc.models[1]->ScalePoints(1,  Vec2f(13.0f, 6.0f), Vec2f(1.0f, 1.2f*fabsf(sin(rotated))));
    //sc.models[1]->ScalePoints(1, sc.models[1]->originalCenter, Vec2f(1.0f, 0.2f + fabsf(sin(rotated2))));
    
    sc.models[0]->TranslatePoints(0, Vec2f(2.0f * sin(rotated2), 2.0f * cos(rotated2)));
    sc.models[0]->TranslatePoints(1, Vec2f(2.0f * cos(rotated2), 2.0f * sin(rotated2)));
    sc.models[0]->TranslatePoints(2, Vec2f(2.0f * sin(rotated2), 2.0f * cos(rotated2)));
    sc.models[0]->TranslatePoints(3, Vec2f(2.0f * cos(rotated2), 2.0f * sin(rotated2)));
    sc.models[0]->TranslatePoints(4, Vec2f(2.0f * sin(rotated2), 2.0f * cos(rotated2)));

    sc.models[0]->Translate(Vec2f(2.5f * cos(rotated) , 2.0f * sin(rotated)));

    /*    sc.models[1]->Rotate(Vec2f(50.0f, 18.0f), rotated2 * 2.0f);
        sc.models[0]->Scale(Vec2f(PANEL_WIDTH/2.0f, PANEL_HEIGHT/2.0f), Vec2f(1.0f + 2.25f * sin(scaled), 1.0f + 2.15f * cos(scaled)));
        
        sc.models[0]->TranslatePoints(0, Vec2f(0.0f, 16.0f * cos(scaled)));
        

        sc.models[0]->Rotate(Vec2f(PANEL_WIDTH/2.0f, PANEL_HEIGHT/2.0f), rotated);
    */
    
    sc.models[2]->Recalculate();
    sc.models[1]->Recalculate();
    sc.models[0]->Recalculate();
    uint32_t diffOp = millis()-start;

    start = millis();
    sc.RenderScene();

    uint32_t diff = millis()-start;
    //start = millis();
    //Devices::Display->clearScreen();
    
    Serial.printf("Transformed in %d and rendered in %d ms (%2.2f fps)\n", diffOp,  diff, 1000.0f/(float)diff);
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
