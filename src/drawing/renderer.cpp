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
    numTriangles = 80;

    triangles = (Trianglef*)malloc(sizeof(Trianglef) * numTriangles);
    

    Serial.printf("done it\n");
    randomizeIt();

    started = true;
   /* 1 1 1 0.5773503 0.5773503 0.5773503 0.625 0.5
-1 1 1 -0.5773503 0.5773503 0.5773503 0.875 0.5
-1 -1 1 -0.5773503 -0.5773503 0.5773503 0.875 0.75
*/
}
void RenderEngine::RenderTriangles() {
    if (!started){
        beginTriangles();
    }
    //Serial.printf("aaaa\n");
    randomizeIt();
    //uint32_t start = millis();
    uint8_t r,g,b;
    Devices::Display->startWrite();
    for (int16_t y=0;y<PANEL_HEIGHT;y++){
      for (int16_t x=0;x<PANEL_WIDTH;x++){  
            int16_t color = Raster(triangles, numTriangles, Vec2f(x,y)); 
            Devices::Display->color565to888(color, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2(x, y, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH)+x, y, r, g, b);
      }
    }
    //Serial.printf("Finished in %d ms\n", millis()-start);
    Devices::Display->endWrite();
}

uint16_t RenderEngine::RasterVec(Trianglef* triangles, int numTriangles, Vec2f pixelRay) {
    float zBuffer = 3.402823466e+38f;
    int triangle = 0;
    bool didIntersect = false;
    float u = 0.0f, v = 0.0f, w = 0.0f;
   // Vector3D uvw;
    uint16_t color = 0;


    for (int i = 0; i < numTriangles; i++){
        subA[i * 2 + 0] = pixelRay.x;//x
        subA[i * 2 + 1] = pixelRay.y;//y

        subB[i * 2 + 0] = triangles[i].p1X;//p1x
        subB[i * 2 + 1] = triangles[i].p1Y;//p1y
    }
    //dsps_sub_f32(const float *input1, const float *input2, float *output, int len, int step1, int step2, int step_out)
    dsps_sub_f32(subA, subB, subDst, numTriangles * 2, 1, 1, 1);

   

    for (int i = 0; i < numTriangles; i++){
        multA[i * 4 + 0] = subDst[i * 2 + 0];//v2lx, from first set
        multA[i * 4 + 1] = triangles[i].v1X;
        multA[i * 4 + 2] = triangles[i].v0X;
        multA[i * 4 + 3] = subDst[i * 2 + 0];//v2lx, from first set
        
        multB[i * 4 + 0] = triangles[i].v1Y;
        multB[i * 4 + 1] = subDst[i * 2 + 1];//v2ly, from first set
        multB[i * 4 + 2] = subDst[i * 2 + 1];//v2ly, from first set
        multB[i * 4 + 3] = triangles[i].v0Y;
    }

    //arm_mult_f32(multA, multB, multDst, numTriangles * 4);
    dsps_mul_f32(multA, multB, multDst, numTriangles * 4, 1, 1, 1);
    //v w subtraction (mult - mult)

    for (int i = 0; i < numTriangles; i++){
        sub1A[i * 2 + 0] = multDst[i * 4];//v2lX * v1Y
        sub1A[i * 2 + 1] = multDst[i * 4 + 2];//v0X * v2lY

        sub1B[i * 2 + 0] = multDst[i * 4 + 1];//v1X * v2lY
        sub1B[i * 2 + 1] = multDst[i * 4 + 3];//v2lX * v0Y
    }

    dsps_sub_f32(sub1A, sub1B, sub1Dst, numTriangles * 2, 1, 1, 1);
    //arm_sub_f32(sub1A, sub1B, sub1Dst, numTriangles * 2);

    //v w denominator multiplication
   
    for (int i = 0; i < numTriangles; i++){
        mult1A[i * 2 + 0] = sub1Dst[i * 2 + 0];//(v2lX * v1Y - v1X * v2lY)
        mult1A[i * 2 + 1] = sub1Dst[i * 2 + 1];//(v0X * v2lY - v2lX * v0Y)
        
        mult1B[i * 2 + 0] = triangles[i].denominator;
        mult1B[i * 2 + 1] = triangles[i].denominator;
    }
    dsps_mul_f32(mult1A, mult1B, mult1Dst, numTriangles * 2, 1, 1, 1);
    //arm_mult_f32(mult1A, mult1B, mult1Dst, numTriangles * 2);//provides v and w

    for (int i = 0; i < numTriangles; i++){
        v = mult1Dst[i * 2 + 0];
        w = mult1Dst[i * 2 + 1];
        u = 1.0f - v - w;

        if (triangles[i].averageDepth < zBuffer) {
            bool intersect = !(v < 0.0f || w < 0.0f || v > 1.0f || w > 1.0f) && u > 0.0f;
            if (intersect){
                //uvw.X = u;
                //uvw.Y = v;
                //uvw.Z = w;
                zBuffer = triangles[i].averageDepth;
                triangle = i;
                didIntersect = true;
            }
        }
    }

    if (didIntersect) {
        //Vector3D intersect = (*triangles[triangle]->t3p1 * uvw.X) + (*triangles[triangle]->t3p2 * uvw.Y) + (*triangles[triangle]->t3p3 * uvw.Z);
        //Vec2f uv;

        //if (triangles[triangle]->hasUV) uv = *triangles[triangle]->p1UV * uvw.X + *triangles[triangle]->p2UV * uvw.Y + *triangles[triangle]->p3UV * uvw.Z;
        color = triangles[triangle].color;
        //color = triangles[triangle]->GetMaterial()->GetRGB(intersect, *triangles[triangle]->normal, Vector3D(uv.X, uv.Y, 0.0f));
    }
    return color;
}

uint16_t RenderEngine::Raster(Trianglef* triangles, int numTriangles, Vec2f pixelRay) {
    float zBuffer = 3.402823466e+38f;
    int triangle = 0;
    bool didIntersect = false;
    float u = 0.0f, v = 0.0f, w = 0.0f;
    //Vector3D uvw;
    uint16_t color = 0;


    for (int t = 0; t < numTriangles; t++) {
        if (triangles[t].averageDepth < zBuffer) {
            if (triangles[t].DidIntersect(pixelRay.x, pixelRay.y, u, v, w)) {
                //uvw.X = u;
                //uvw.Y = v;
                //uvw.Z = w;
                zBuffer = triangles[t].averageDepth;
                triangle = t;
                didIntersect = true;
                break;
            }
        }
    }
    
    if (didIntersect) {
        //Vector3D intersect = (*triangles[triangle]->t3p1 * uvw.X) + (*triangles[triangle]->t3p2 * uvw.Y) + (*triangles[triangle]->t3p3 * uvw.Z);
        //Vec2f uv;

        //if (triangles[triangle]->hasUV) uv = *triangles[triangle]->p1UV * uvw.X + *triangles[triangle]->p2UV * uvw.Y + *triangles[triangle]->p3UV * uvw.Z;
        color = triangles[triangle].color;
        //color = triangles[triangle]->GetMaterial()->GetRGB(intersect, *triangles[triangle]->normal, Vector3D(uv.X, uv.Y, 0.0f));
    }

    return color;
}


/*
RGBColor Rasterizer::CheckRasterPixel(Trianglef** triangles, int numTriangles, Vec2f pixelRay) {
    float zBuffer = 3.402823466e+38f;
    int triangle = 0;
    bool didIntersect = false;
    float u = 0.0f, v = 0.0f, w = 0.0f;
    Vector3D uvw;
    RGBColor color;

    for (int t = 0; t < numTriangles; ++t) {
        if (triangles[t]->averageDepth < zBuffer) {
            if (triangles[t]->DidIntersect(pixelRay.X, pixelRay.Y, u, v, w)) {
                uvw.X = u;
                uvw.Y = v;
                uvw.Z = w;
                zBuffer = triangles[t]->averageDepth;
                triangle = t;
                didIntersect = true;
            }
        }
    }
    
    if (didIntersect) {
        Vector3D intersect = (*triangles[triangle]->t3p1 * uvw.X) + (*triangles[triangle]->t3p2 * uvw.Y) + (*triangles[triangle]->t3p3 * uvw.Z);
        Vec2f uv;

        if (triangles[triangle]->hasUV) uv = *triangles[triangle]->p1UV * uvw.X + *triangles[triangle]->p2UV * uvw.Y + *triangles[triangle]->p3UV * uvw.Z;

        color = triangles[triangle]->GetMaterial()->GetRGB(intersect, *triangles[triangle]->normal, Vector3D(uv.X, uv.Y, 0.0f));
    }

    return color;
}

void Rasterizer::Rasterize(Scene* scene, CameraBase* camera) {
    if (camera->Is2D()) {
        for (unsigned int i = 0; i < camera->GetPixelGroup()->GetPixelCount(); i++) {
            Vec2f pixelRay = camera->GetPixelGroup()->GetCoordinate(i);
            Vector3D pixelRay3D = Vector3D(pixelRay.X, pixelRay.Y, 0) + camera->GetTransform()->GetPosition();

            RGBColor color = scene->GetObjects()[0]->GetMaterial()->GetRGB(pixelRay3D, Vector3D(), Vector3D());

            camera->GetPixelGroup()->GetColor(i)->R = color.R;
            camera->GetPixelGroup()->GetColor(i)->G = color.G;
            camera->GetPixelGroup()->GetColor(i)->B = color.B;
        }
    } else {
        camera->GetTransform()->SetBaseRotation(camera->GetCameraLayout()->GetRotation());//Set camera layout

        rayDirection = camera->GetTransform()->GetRotation().Multiply(camera->GetLookOffset());//Apply offset to camera if set

        //Set quad tree space to local camera coordinates
        Vec2f minCoord = camera->GetCameraMinCoordinate();
        Vec2f maxCoord = camera->GetCameraMaxCoordinate();

        QuadTree tree = QuadTree(minCoord, maxCoord);

        uint16_t tCount = 1;

        for (uint8_t i = 0; i < scene->GetObjectCount(); ++i) {
            if (scene->GetObjects()[i]->IsEnabled()) {
                tCount += scene->GetObjects()[i]->GetTriangleGroup()->GetTriangleCount();
            }
        }

        Trianglef triangles[tCount];
        uint16_t iterCount = 0;

        for (uint8_t i = 0; i < scene->GetObjectCount(); ++i) {
            if (scene->GetObjects()[i]->IsEnabled()) {
                for (uint16_t j = 0; j < scene->GetObjects()[i]->GetTriangleGroup()->GetTriangleCount(); ++j) {
                    //Create 2D triangle mapping for rasterize, stores 3D coordinates for mapping material to 3d global coordinate space
                    triangles[iterCount] = Trianglef(camera->GetLookOffset(), camera->GetTransform(), &scene->GetObjects()[i]->GetTriangleGroup()->GetTriangles()[j], scene->GetObjects()[i]->GetMaterial());
                    
                    tree.Insert(&triangles[iterCount]);
                    iterCount++;
                }
            }
        }

        tree.Rebuild();

        Vec2f scale, pixelRay, materialRay;
        for (uint16_t i = 0; i < camera->GetPixelGroup()->GetPixelCount(); ++i) {
            //Render camera in local camera space
            pixelRay = camera->GetPixelGroup()->GetCoordinate(i);
            
            Node* leafNode = tree.Intersect(pixelRay);

            if (!leafNode || leafNode->GetCount() == 0) {
                camera->GetPixelGroup()->GetColor(i)->R = 0;
                camera->GetPixelGroup()->GetColor(i)->G = 0;
                camera->GetPixelGroup()->GetColor(i)->B = 0;
                continue;
            }

            //Render individual pixel, transformed to camera coordinate space
            RGBColor color = CheckRasterPixel(leafNode->GetEntities(), leafNode->GetCount(), pixelRay);

            camera->GetPixelGroup()->GetColor(i)->R = color.R;
            camera->GetPixelGroup()->GetColor(i)->G = color.G;
            camera->GetPixelGroup()->GetColor(i)->B = color.B;
        }
    }
}*/