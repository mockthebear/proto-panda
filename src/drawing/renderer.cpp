#include "drawing/renderer.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

void RenderEngine::randomizeIt(){
    

}

void RenderEngine::beginTriangles(){
    

    Serial.printf("done it\n");
    Model *mem = new Model();
    numTriangles = 64;
    mem->Begin(numTriangles);
    mem->SetTriangle(0, Vec2f(5.0f,19.0f), Vec2f(8.0f,22.0f), Vec2f(12.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(1, Vec2f(5.0f,19.0f), Vec2f(8.0f,18.0f), Vec2f(12.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(2, Vec2f(7.0f,18.0f), Vec2f(9.0f,18.0f), Vec2f(8.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(3, Vec2f(11.0f,21.0f), Vec2f(18.0f,27.0f), Vec2f(21.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(4, Vec2f(11.0f,21.0f), Vec2f(14.0f,21.0f), Vec2f(21.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(5, Vec2f(18.0f,27.0f), Vec2f(26.0f,20.0f), Vec2f(28.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(6, Vec2f(18.0f,27.0f), Vec2f(21.0f,27.0f), Vec2f(28.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(7, Vec2f(26.0f,20.0f), Vec2f(33.0f,27.0f), Vec2f(35.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(8, Vec2f(26.0f,20.0f), Vec2f(28.0f,20.0f), Vec2f(35.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(9, Vec2f(33.0f,27.0f), Vec2f(35.0f,27.0f), Vec2f(42.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(10, Vec2f(42.0f,20.0f), Vec2f(40.0f,20.0f), Vec2f(33.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(11, Vec2f(40.0f,20.0f), Vec2f(48.0f,27.0f), Vec2f(50.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(12, Vec2f(40.0f,20.0f), Vec2f(42.0f,20.0f), Vec2f(50.0f,27.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(13, Vec2f(60.0f,5.0f), Vec2f(55.0f,5.0f), Vec2f(60.0f,8.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(14, Vec2f(7.0f,6.0f), Vec2f(11.0f,8.0f), Vec2f(14.0f,6.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(15, Vec2f(7.0f,6.0f), Vec2f(9.0f,5.0f), Vec2f(12.0f,5.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(16, Vec2f(12.0f,5.0f), Vec2f(14.0f,6.0f), Vec2f(16.0f,5.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(17, Vec2f(9.0f,5.0f), Vec2f(16.0f,5.0f), Vec2f(13.0f,3.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(18, Vec2f(14.0f,6.0f), Vec2f(16.0f,5.0f), Vec2f(17.0f,6.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(19, Vec2f(11.0f,8.0f), Vec2f(14.0f,6.0f), Vec2f(17.0f,8.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(20, Vec2f(17.0f,8.0f), Vec2f(17.0f,6.0f), Vec2f(14.0f,6.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(21, Vec2f(17.0f,6.0f), Vec2f(18.0f,7.0f), Vec2f(17.0f,8.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(22, Vec2f(12.0f,20.0f), Vec2f(11.0f,21.0f), Vec2f(9.0f,18.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(23, Vec2f(12.0f,20.0f), Vec2f(11.0f,21.0f), Vec2f(8.0f,20.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(24, Vec2f(26.0f,3.0f), Vec2f(27.0f,5.0f), Vec2f(30.0f,3.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(25, Vec2f(33.0f,3.0f), Vec2f(33.0f,6.0f), Vec2f(32.0f,8.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(26, Vec2f(35.0f,7.0f), Vec2f(36.0f,5.0f), Vec2f(38.0f,4.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(27, Vec2f(41.0f,6.0f), Vec2f(42.0f,9.0f), Vec2f(41.0f,11.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(28, Vec2f(36.0f,12.0f), Vec2f(34.0f,13.0f), Vec2f(30.0f,14.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(29, Vec2f(23.0f,11.0f), Vec2f(22.0f,13.0f), Vec2f(24.0f,16.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(30, Vec2f(31.0f,17.0f), Vec2f(36.0f,17.0f), Vec2f(39.0f,14.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(31, Vec2f(43.0f,13.0f), Vec2f(49.0f,14.0f), Vec2f(51.0f,17.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(32, Vec2f(49.0f,21.0f), Vec2f(54.0f,23.0f), Vec2f(58.0f,21.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(33, Vec2f(59.0f,17.0f), Vec2f(57.0f,15.0f), Vec2f(54.0f,12.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(34, Vec2f(50.0f,8.0f), Vec2f(49.0f,5.0f), Vec2f(51.0f,2.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(35, Vec2f(53.0f,1.0f), Vec2f(55.0f,1.0f), Vec2f(58.0f,2.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(36, Vec2f(61.0f,3.0f), Vec2f(63.0f,5.0f), Vec2f(63.0f,10.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(37, Vec2f(62.0f,14.0f), Vec2f(62.0f,18.0f), Vec2f(61.0f,23.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(38, Vec2f(61.0f,25.0f), Vec2f(59.0f,28.0f), Vec2f(53.0f,29.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(39, Vec2f(50.0f,30.0f), Vec2f(45.0f,31.0f), Vec2f(39.0f,30.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(40, Vec2f(35.0f,29.0f), Vec2f(30.0f,29.0f), Vec2f(26.0f,29.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(41, Vec2f(20.0f,29.0f), Vec2f(13.0f,29.0f), Vec2f(6.0f,29.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(42, Vec2f(2.0f,27.0f), Vec2f(1.0f,24.0f), Vec2f(0.0f,20.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(43, Vec2f(1.0f,14.0f), Vec2f(4.0f,12.0f), Vec2f(8.0f,12.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(44, Vec2f(13.0f,15.0f), Vec2f(16.0f,17.0f), Vec2f(17.0f,12.0f), Devices::Display->color565(255,255,255));
    mem->SetTriangle(45, Vec2f(17.0f,11.0f), Vec2f(2.0f,2.0f), Vec2f(1.0f,4.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(46, Vec2f(2.0f,6.0f), Vec2f(2.0f,9.0f), Vec2f(5.0f,9.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(47, Vec2f(23.0f,7.0f), Vec2f(25.0f,4.0f), Vec2f(21.0f,1.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(48, Vec2f(19.0f,1.0f), Vec2f(16.0f,2.0f), Vec2f(13.0f,0.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(49, Vec2f(24.0f,7.0f), Vec2f(25.0f,11.0f), Vec2f(27.0f,9.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(50, Vec2f(30.0f,11.0f), Vec2f(29.0f,13.0f), Vec2f(26.0f,14.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(51, Vec2f(31.0f,19.0f), Vec2f(33.0f,21.0f), Vec2f(34.0f,21.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(52, Vec2f(38.0f,18.0f), Vec2f(42.0f,16.0f), Vec2f(42.0f,16.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(53, Vec2f(45.0f,18.0f), Vec2f(46.0f,20.0f), Vec2f(46.0f,21.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(54, Vec2f(49.0f,19.0f), Vec2f(51.0f,19.0f), Vec2f(52.0f,18.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(55, Vec2f(46.0f,7.0f), Vec2f(45.0f,9.0f), Vec2f(44.0f,11.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(56, Vec2f(48.0f,10.0f), Vec2f(50.0f,13.0f), Vec2f(51.0f,10.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(57, Vec2f(41.0f,2.0f), Vec2f(42.0f,5.0f), Vec2f(44.0f,2.0f), Devices::Display->color565(0,255,0));
    mem->SetTriangle(58, Vec2f(6.0f,25.0f), Vec2f(7.0f,25.0f), Vec2f(7.0f,25.0f), Devices::Display->color565(0,0,255));
    mem->SetTriangle(59, Vec2f(2.0f,17.0f), Vec2f(4.0f,16.0f), Vec2f(8.0f,15.0f), Devices::Display->color565(255,0,0));
    mem->SetTriangle(60, Vec2f(0.0f,29.0f), Vec2f(2.0f,30.0f), Vec2f(3.0f,28.0f), Devices::Display->color565(255,0,0));
    mem->SetTriangle(61, Vec2f(40.0f,26.0f), Vec2f(41.0f,28.0f), Vec2f(42.0f,26.0f), Devices::Display->color565(255,0,0));
    mem->SetTriangle(62, Vec2f(17.0f,18.0f), Vec2f(18.0f,19.0f), Vec2f(20.0f,16.0f), Devices::Display->color565(255,0,0));
    mem->SetTriangle(63, Vec2f(36.0f,8.0f), Vec2f(38.0f,11.0f), Vec2f(39.0f,8.0f), Devices::Display->color565(255,0,0));
    mem->Recalculate();
    mem->SetBatchOperations(true);
    mem->SetAccumulativeOperations(true);

    mem->AddPointGroup(std::vector<int>({17, 16, 20, 25, 24,21}));


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
    static float incrRotated = (0.1f * M_PI / 180.0f);
    rotated += incrRotated;

    static float rotated2 = 0.0f;
    static float incrRotated2 = (-2.0f * M_PI / 180.0f);
    rotated2 += incrRotated2;
    static bool shouldApply = false;

    if (rotated > (15.0f * M_PI / 180.0f)){
        incrRotated = -(0.1f * M_PI / 180.0f);
        rotated = (15.0f * M_PI / 180.0f);
    }else if(rotated < (-15.0f * M_PI / 180.0f)){
        rotated = (-15.0f * M_PI / 180.0f);
        incrRotated = (0.1f * M_PI / 180.0f);
        shouldApply = true;
    }

    scaled += increment;

    //
    uint32_t start = millis();
    sc.models[0]->Reset();
    if (shouldApply){
        
        sc.models[0]->Rotate(Vec2f(PANEL_WIDTH/2.0f, PANEL_HEIGHT/2.0f), rotated);
        sc.models[0]->Scale(Vec2f(PANEL_WIDTH/2.0f, PANEL_HEIGHT/2.0f), Vec2f(1.0f + 0.25f * sin(scaled), 1.0f + 0.15f * cos(scaled)));
        
        //sc.models[0]->TranslatePoints(0, Vec2f(0.0f, 16.0f * cos(scaled)));
        sc.models[0]->Translate(Vec2f(10.0f * sin(rotated2), 10.0f * cos(rotated2)));
        
    }
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
