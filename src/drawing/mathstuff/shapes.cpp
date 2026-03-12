#include "drawing/mathstuff/shapes.hpp"
#include "tools/devices.hpp"


void PointGroups::Translate(uint32_t group, Vec2f position){
    if (group >= groupCount){
        return;
    }
    auto &it = points[group];
    auto xPoints = mainModel->originalPoints.x;
    auto yPoints = mainModel->originalPoints.y;
    if (mainModel->accumulatedOperation){
        xPoints = mainModel->points.x;
        yPoints = mainModel->points.y;
    }
    for (auto &pointid : it){
        xPoints[pointid] += position.x;
        yPoints[pointid] += position.y;
    }
};

void PointGroups::Set(uint32_t group, Vec2f position){
    if (group >= groupCount){
        return;
    }
    auto &it = points[group];
    auto xPoints = mainModel->originalPoints.x;
    auto yPoints = mainModel->originalPoints.y;
    if (mainModel->accumulatedOperation){
        xPoints = mainModel->points.x;
        yPoints = mainModel->points.y;
    }
    for (auto &pointid : it){
        xPoints[pointid] = position.x;
        yPoints[pointid] = position.y;
    }
};


bool Model::Begin(int sz){
    triangleCount = sz;
    points.Allocate(triangleCount * 3 );
    originalPoints.Allocate(triangleCount * 3 );
    int allocateSize = triangleCount * 5 + ((triangleCount * 3) * 3);
    denominators =  (float*)heap_caps_malloc(allocateSize * sizeof(float), MALLOC_CAP_SPIRAM); 
    if (denominators == nullptr){
        points.Deallocate();
        originalPoints.Deallocate();
        triangleCount = 0;
        return false;
    }
    v0X = denominators + triangleCount * 1;
    v0Y = denominators + triangleCount * 2;
    v1X = denominators + triangleCount * 3;
    v1Y = denominators + triangleCount * 4;
    aux1 = denominators +((triangleCount * 5) + ((triangleCount * 3) * 0));
    aux2 = denominators +((triangleCount * 5) + ((triangleCount * 3) * 1));
    aux3 = denominators +((triangleCount * 5) + ((triangleCount * 3) * 2));
    color = (uint16_t*)heap_caps_malloc(sizeof(uint16_t) * triangleCount, MALLOC_CAP_SPIRAM);
    if (color == nullptr){
        originalPoints.Deallocate();
        points.Deallocate();
        heap_caps_free(denominators);
        triangleCount = 0;
        denominators = v0X = v0Y = v1X = v1Y = aux1 = aux2 = aux3 = nullptr;
        color = nullptr;
        return false;
    }
    return true;
}

void Model::Free(){
    if (triangleCount == 0){
        return;
    }
    originalPoints.Deallocate();
    points.Deallocate();
    heap_caps_free(denominators);
    heap_caps_free(color);
    triangleCount = 0;
    denominators = v0X = v0Y = v1X = v1Y = aux1 = aux2 = aux3 = nullptr;
    color = nullptr;
}


void Model::Translate(Vec2f delta) {
    if (triangleCount == 0) {
        return;
    }
    auto xPoints = this->originalPoints.x;
    auto yPoints = this->originalPoints.y;
    
    if (accumulatedOperation){
        xPoints = this->points.x;
        yPoints = this->points.y;
    }

    const int vecSize = triangleCount * 3;
    dsps_addc_f32(xPoints, this->points.x, vecSize, delta.x, 1, 1);
    dsps_addc_f32(yPoints, this->points.y, vecSize, delta.y, 1, 1);
    
    if (!batchOperations){
        Recalculate();
    }
}

void Model::Rotate(Vec2f center, float angle){
    if (triangleCount == 0){
        return;
    }
    //esp_err_t dsps_mulc_f32_ae32(const float *input, float *output, int len, float C, int step_in, int step_out);
    // x[i*step_out] = y[i*step_in]*C; i=[0..len)
    const int vecSize = triangleCount * 3;

    float commonCos = cos(angle);
    float commonSin = sin(angle);

    auto xPoints = this->originalPoints.x;
    auto YPoints = this->originalPoints.y;
            
    if (accumulatedOperation){
        xPoints = this->points.x;
        YPoints = this->points.y;
    }


    //float translatedX = point.x - center.x;
    dsps_addc_f32(xPoints, this->points.x, vecSize, -center.x, 1, 1);
    //float translatedY = point.y - center.y;
    dsps_addc_f32(YPoints, this->points.y, vecSize, -center.y, 1, 1);
            
    // point.x = (this->points.x * cos - this->points.y * sin) + center.x
    dsps_mulc_f32(this->points.x, aux1, vecSize, commonCos, 1, 1);
    dsps_mulc_f32(this->points.y, aux2, vecSize, -commonSin, 1, 1);
    dsps_add_f32(aux1, aux2, aux3, vecSize, 1, 1, 1);
            

    // point.y = (this->points.x * sin + this->points.y * cos) + center.y
    dsps_mulc_f32(this->points.x, aux1, vecSize, commonSin, 1, 1);
    dsps_mulc_f32(this->points.y, aux2, vecSize, commonCos, 1, 1);
    dsps_add_f32(aux1, aux2, aux1, vecSize, 1, 1, 1);
    dsps_addc_f32(aux1, this->points.y, vecSize, center.y, 1, 1);
    dsps_addc_f32(aux3, this->points.x, vecSize, center.x, 1, 1);
    if (!batchOperations){
        Recalculate();
    }
};


void Model::Reset() {
    const int vecSize = triangleCount * 3;
    dsps_addc_f32(this->originalPoints.x, this->points.x, vecSize, 0, 1, 1);
    dsps_addc_f32(this->originalPoints.y, this->points.y, vecSize, 0, 1, 1);
    if (!batchOperations){
        Recalculate();
    }
}

void Model::Store() {
    const int vecSize = triangleCount * 3;
    dsps_addc_f32(this->points.x, this->originalPoints.x, vecSize, 0, 1, 1);
    dsps_addc_f32(this->points.y, this->originalPoints.y, vecSize, 0, 1, 1);
    if (!batchOperations){
        Recalculate();
    }
}

void Model::Scale(Vec2f center, Vec2f scaleFactors) {
    if (triangleCount == 0) {
        return;
    }
    
    const int vecSize = triangleCount * 3;

    auto xPoints = this->originalPoints.x;
    auto yPoints = this->originalPoints.y;
            
    if (accumulatedOperation){
        xPoints = this->points.x;
        yPoints = this->points.y;
    }
            
    dsps_addc_f32(xPoints, this->points.x, vecSize, -center.x, 1, 1);
    dsps_addc_f32(yPoints, this->points.y, vecSize, -center.y, 1, 1);
            
    dsps_mulc_f32(this->points.x, this->points.x, vecSize, scaleFactors.x, 1, 1);
    dsps_mulc_f32(this->points.y, this->points.y, vecSize, scaleFactors.y, 1, 1);
            
    dsps_addc_f32(this->points.x, this->points.x, vecSize, center.x, 1, 1);
    dsps_addc_f32(this->points.y, this->points.y, vecSize, center.y, 1, 1);
            
    if (!batchOperations){
        Recalculate();
    }
}

void Model::Recalculate(){            
    if (triangleCount == 0){
        return;
    }
    //dsps_sub_f32(input1    , input2  , *output, len          , step1, step2, step_out);
    //out[i*step_out] = input1[i*step1] - input2[i*step2]; i=[0..len)
    //For 0, 3
    //v0X[0] = points.x[0+1] - points.x[localIndex];
    //v0X[1] = points.x[3+1] - points.x[localIndex];
    //v0X[2] = points.x[6+1] - points.x[localIndex];
    dsps_sub_f32(points.x+1, points.x, v0X    , triangleCount,     3,     3, 1);
    //v0Y[i] = points.y[localIndex+1] - points.y[localIndex];
    dsps_sub_f32(points.y+1, points.y, v0Y    , triangleCount,     3,     3, 1);
    //v1X[i] = points.x[localIndex+2] - points.x[localIndex];
    dsps_sub_f32(points.x+2, points.x, v1X    , triangleCount,     3,     3, 1);
    //v1Y[i] = points.y[localIndex+2] - points.y[localIndex];
    dsps_sub_f32(points.y+2, points.y, v1Y    , triangleCount,     3,     3, 1);

    //aux1 = v0X[i] * v1Y[i]
    dsps_mul_f32(v0X, v1Y, aux1, triangleCount, 1, 1, 1);
    //aux2 = v0X[i] * v0Y[i]
    dsps_mul_f32(v1X, v0Y, aux2, triangleCount, 1, 1, 1);
    //aux1 = ((aux1[i]) - (aux2[i]));
    dsps_sub_f32(aux1, aux2, aux1    , triangleCount,     1,     1, 1);
      
    boundaries.x[1] = 999999.0f;
    boundaries.y[1] = 999999.0f;
    boundaries.x[0] = -999999.0f;
    boundaries.y[0] = -999999.0f;
    for (int i=0;i<triangleCount;i++){
        if (aux1[i] != 0.0f){
            denominators[i] = 1.0f / aux1[i];
        }else{
            denominators[i] = 0.0f;
        }
        for (int j = 0; j < 3; j++){
            //Calculate max and mins
            int pointIdx = j+i*3;
            if (points.x[pointIdx] > boundaries.x[0] ){
                boundaries.x[0] = points.x[pointIdx];
            }
            if (points.y[pointIdx] > boundaries.y[0] ){
                boundaries.y[0] = points.y[pointIdx];
            }

            if (points.x[pointIdx] < boundaries.x[1]){
                boundaries.x[1] = points.x[pointIdx];
            }
            if (points.y[pointIdx] < boundaries.y[1]){
                boundaries.y[1] = points.y[pointIdx];
            }
        }
    }
    center.x = (boundaries.x[0]-boundaries.x[1])/2.0f;
    center.y = (boundaries.y[0]-boundaries.y[1])/2.0f;
}

Trianglef Model::GetTriangle(int i){
    if (i > triangleCount){
        return Trianglef();
    }
    Trianglef aux;
    int localIndex = i * 3;
    aux.points.x[0] = originalPoints.x[localIndex];
    aux.points.y[0] = originalPoints.y[localIndex];
    aux.points.x[1] = originalPoints.x[localIndex+1];
    aux.points.y[1] = originalPoints.y[localIndex+1];
    aux.points.x[2] = originalPoints.x[localIndex+2];
    aux.points.y[2] = originalPoints.y[localIndex+2];

    aux.v0X         = v0X[i];
    aux.v0Y         = v0Y[i];
    aux.v1X         = v1X[i];
    aux.v1Y         = v1Y[i];
    aux.denominator = denominators[i];
    aux.color       = color[i];

    return aux;
};

bool Model::DidIntersect(const float& x, const float& y, float& u, float& v, float& w, uint16_t &colorOut){
    if (x > boundaries.x[0]){
        return false;
    }
    if (y > boundaries.y[0]){
        return false;
    }
    if (x < boundaries.x[1]){
        return false;
    }
    if (y < boundaries.y[1]){
        return false;
    }
   
    for (int i=0;i<triangleCount;i++){
        if (denominators[i] == 0.0f){
            continue;
        }
        int tringleId = i * 3;

        float v2lX = x - points.x[tringleId];
        float v2lY = y - points.y[tringleId];

        v = (v2lX * v1Y[i] - v1X[i] * v2lY) * denominators[i];
        if (v < 0.0f || v > 1.0f){
            continue;
        }
        w = (v0X[i] * v2lY - v2lX * v0Y[i]) * denominators[i];

        if (w < 0.0f || w > 1.0f){
            continue;
        }
        //Dont change to 1.0f That tiny decimal part makes thing looks good.
        u = 1.0001f - v - w;

        if (u >= 0.0f){
            colorOut = color[i];
            return true;
        }
    }

    return false;
}

void Model::SetTriangle(int i, Vec2f p1, Vec2f p2, Vec2f p3, uint16_t color){
    if (i > triangleCount){
        return;
    }
    int localIndex = i * 3;
    originalPoints.x[localIndex  ]          = p1.x;
    originalPoints.y[localIndex  ]          = p1.y;
    originalPoints.x[localIndex+1]          = p2.x;
    originalPoints.y[localIndex+1]          = p2.y;
    originalPoints.x[localIndex+2]          = p3.x;
    originalPoints.y[localIndex+2]          = p3.y;
    this->color[i] = color;
}

void Model::SetTriangle(int i, Trianglef aux){
    if (i > triangleCount){
        return;
    }

    int localIndex = i * 3;
    originalPoints.x[localIndex  ]          = aux.points.x[0];
    originalPoints.y[localIndex  ]          = aux.points.y[0];
    originalPoints.x[localIndex+1]          = aux.points.x[1];
    originalPoints.y[localIndex+1]          = aux.points.y[1];
    originalPoints.x[localIndex+2]          = aux.points.x[2];
    originalPoints.y[localIndex+2]          = aux.points.y[2];
    color[i] = aux.color;
}

int Model::AddPointGroup(std::vector<int> pts) {
    bones.points.emplace_back(pts);
    bones.groupCount++;
    return bones.points.size() - 1; 
}

inline bool Scene::RenderModels(const float& x, const float& y, uint16_t &colorOut){
    float u = 0.0f, v = 0.0f, w = 0.0f;
    for (auto &it : models){
        if (it->DidIntersect(x, y, u, v, w, colorOut)){
            return true;
        }
    }
    return false;
}

void Model::TranslatePoints(uint32_t groupid, Vec2f delta){
    bones.Translate(groupid, delta);
}
void Model::SetPointsPosition(uint32_t groupid, Vec2f pos){
    bones.Set(groupid, pos);
}


void Scene::RenderScene(){
    Devices::Display->startWrite();
    uint8_t r,g,b;
    for (float y=0;y<PANEL_HEIGHT;y++ ){
      for (float x=0;x<PANEL_WIDTH;x++){  
            uint16_t color = 0;
            RenderModels(x,y,color);
            //Raster(triangles, numTriangles, Vec2f(x,y)); 
            Devices::Display->color565to888(color, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2(x, y, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH)+x, y, r, g, b);
      }
    }
    Devices::Display->endWrite();
}

