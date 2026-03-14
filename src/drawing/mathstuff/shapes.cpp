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

void PointGroups::Scale(uint32_t group, Vec2f center, Vec2f scaleFactors){
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
        // Translate point to origin relative to center
        float x = xPoints[pointid] - center.x;
        float y = yPoints[pointid] - center.y;
        
        // Scale
        x *= scaleFactors.x;
        y *= scaleFactors.y;
        
        // Translate back
        xPoints[pointid] = x + center.x;
        yPoints[pointid] = y + center.y;
    }
}

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
    rasterPoints.Allocate(triangleCount * 3 );
    int allocateSize = (triangleCount * 3) * 3;
    aux1 =  (float*)heap_caps_malloc(allocateSize * sizeof(float), MALLOC_CAP_SPIRAM); 
    if (aux1 == nullptr){
        points.Deallocate();
        originalPoints.Deallocate();
        rasterPoints.Deallocate();
        triangleCount = 0;
        return false;
    }
    aux2 = aux1 + (((triangleCount * 3) * 1));
    aux3 = aux1 + (((triangleCount * 3) * 2));

    color = (uint16_t*)heap_caps_malloc(sizeof(uint16_t) * triangleCount, MALLOC_CAP_SPIRAM);
    if (color == nullptr){
        rasterPoints.Deallocate();
        originalPoints.Deallocate();
        points.Deallocate();
        heap_caps_free(aux1);
        triangleCount = 0;
        aux1 = aux2 = aux3 = nullptr;
        color = nullptr;
        return false;
    }

    originalBoundaries.x[1] = 999999.0f;
    originalBoundaries.y[1] = 999999.0f;
    originalBoundaries.x[0] = -999999.0f;
    originalBoundaries.y[0] = -999999.0f;
    return true;
}

void Model::Free(){
    if (triangleCount == 0){
        return;
    }
    originalPoints.Deallocate();
    rasterPoints.Deallocate();
    points.Deallocate();
    heap_caps_free(aux1);
    heap_caps_free(color);
    triangleCount = 0;
    aux1 = aux2 = aux3 = nullptr;
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

void Model::CopyToRaster() {
    const int vecSize = triangleCount * 3;
    dsps_addc_f32(this->points.x, this->rasterPoints.x, vecSize, 0, 1, 1);
    dsps_addc_f32(this->points.y, this->rasterPoints.y, vecSize, 0, 1, 1);
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
      
    boundaries.x[1] = 999999.0f;
    boundaries.y[1] = 999999.0f;
    boundaries.x[0] = -999999.0f;
    boundaries.y[0] = -999999.0f;
    for (int i=0;i<triangleCount;i++){
        int baseIdx = i * 3;
        
        // Get the three vertices
        Vec2f v0 = {points.x[baseIdx], points.y[baseIdx]};
        Vec2f v1 = {points.x[baseIdx + 1], points.y[baseIdx + 1]};
        Vec2f v2 = {points.x[baseIdx + 2], points.y[baseIdx + 2]};
        
        // Sort them by y-coordinate (v0.y <= v1.y <= v2.y)
        // We'll use a simple sorting network for three elements
        if (v0.y > v1.y) {
            std::swap(v0, v1);
        }
        if (v0.y > v2.y) {
            std::swap(v0, v2);
        }
        if (v1.y > v2.y) {
            std::swap(v1, v2);
        }
        
        // Write back sorted vertices
        points.x[baseIdx] = v0.x;
        points.y[baseIdx] = v0.y;
        points.x[baseIdx + 1] = v1.x;
        points.y[baseIdx + 1] = v1.y;
        points.x[baseIdx + 2] = v2.x;
        points.y[baseIdx + 2] = v2.y;

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
    return aux;
};

void Model::SetTriangle(int i, Vec2f p1, Vec2f p2, Vec2f p3, uint16_t color){
    if (i >= triangleCount){
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

    for (int idx = localIndex; idx < localIndex+3; idx++ ){
        if (originalPoints.x[idx] > originalBoundaries.x[0] ){
            originalBoundaries.x[0] = originalPoints.x[idx];
        }
        if (originalPoints.y[idx] > originalBoundaries.y[0] ){
            originalBoundaries.y[0] = originalPoints.y[idx];
        }

        if (originalPoints.x[idx] < originalBoundaries.x[1]){
            originalBoundaries.x[1] = originalPoints.x[idx];
        }
        if (originalPoints.x[idx] < originalBoundaries.y[1]){
            originalBoundaries.y[1] = originalPoints.y[idx];
        }
    }
    originalCenter.x = (originalBoundaries.x[0]-originalBoundaries.x[1])/2.0f;
    originalCenter.y = (originalBoundaries.y[0]-originalBoundaries.y[1])/2.0f;
}

void Model::SetTriangle(int i, Trianglef aux){
    if (i >= triangleCount){
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

int Model::AddPointGroup(PointList &pts) {
    bones.points.emplace_back(pts);
    bones.groupCount++;
    return bones.points.size() - 1; 
}
void Model::RasterTriangle(Scene *scene, int i){

    int baseIdx = i * 3;

    int16_t x0 = rasterPoints.x[baseIdx + 0]; 
    int16_t y0 = rasterPoints.y[baseIdx + 0]; 
    int16_t x1 = rasterPoints.x[baseIdx + 1]; 
    int16_t y1 = rasterPoints.y[baseIdx + 1];
    int16_t x2 = rasterPoints.x[baseIdx + 2]; 
    int16_t y2 = rasterPoints.y[baseIdx + 2]; 
    uint16_t color = this->color[i];
    int16_t a, b, y, last;
  
    uint8_t r, g, bl;
    Devices::Display->color565to888(color, r, g, bl);

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
        a = x1;
        else if (x1 > b)
        b = x1;
        if (x2 < a)
        a = x2;
        else if (x2 > b)
        b = x2;
        if (a < 0){
            a = 0;
        }
        if (y0 < 0 || y0 >= PANEL_HEIGHT){
            return;
        }
        for (;a <= b; a++) {
            if (a >= PANEL_WIDTH){
                break;
            }
            if (!scene->MarkPixel(a, y0)) {
                Devices::Display->updateMatrixDMABuffer_2(a, y0, r, g, bl);
                Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH+PANEL_WIDTH-1) - a, y0, r, g, bl);
            }
        }
        return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0;
    int16_t dx02 = x2 - x0, dy02 = y2 - y0;
    int16_t dx12 = x2 - x1, dy12 = y2 - y1;
    int16_t sa = 0, sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1; // Include y1 scanline
    else
        last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if (a > b){
            float t = a;
            a = b;
            b = t;
        }
        if (a < 0){
            a = 0;
        }
        if (y < 0 || y >= PANEL_HEIGHT){
            continue;
        }
        for (int xx = a; xx <= b; xx++) {
            if (xx >= PANEL_WIDTH){
                break;
            }
            if (!scene->MarkPixel(xx, y)) {
                Devices::Display->updateMatrixDMABuffer_2(xx, y, r, g, bl);
                Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH+PANEL_WIDTH-1) - xx, y, r, g, bl);
            }
        }
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (float)dx12 * (y - y1);
    sb = (float)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;

        if (a > b){
            float t = a;
            a = b;
            b = t;
        }
        if (a < 0){
            a = 0;
        }
        if (y < 0 || y >= PANEL_HEIGHT){
            continue;
        }
        for (int xx = a; xx <= b; xx++) {
            if (xx >= PANEL_WIDTH){
                break;
            }
            if (!scene->MarkPixel(xx, y)) {
                Devices::Display->updateMatrixDMABuffer_2(xx, y, r, g, bl);
                Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH+PANEL_WIDTH-1) - xx, y, r, g, bl);
            }
        }
    }
}


void Scene::RenderModels(){
    for (auto& model : models) {
        if (model->triangleCount == 0) continue;
        
        for (int i = model->triangleCount - 1; i >= 0; i--) {
            model->RasterTriangle(this, i);
        }
    }
}

void Model::TranslatePoints(uint32_t groupid, Vec2f delta){
    bones.Translate(groupid, delta);
}

void Model::ScalePoints(uint32_t groupid, Vec2f center, Vec2f scaleFactors){
    bones.Scale(groupid, center, scaleFactors);
}
void Model::SetPointsPosition(uint32_t groupid, Vec2f pos){
    bones.Set(groupid, pos);
}

void Scene::RenderScene(){
    Devices::Display->startWrite();
    memset(pixelBitmap, 0, sizeof(pixelBitmap));
    uint8_t r, g, b;
    Devices::Display->color565to888(0, r, g, b);
    for (float y = 0; y < PANEL_HEIGHT; y++) {
        for (float x = 0; x < PANEL_WIDTH; x++) {
            Devices::Display->updateMatrixDMABuffer_2(x, y, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH) + x, y, r, g, b);
        }
    }
    RenderModels();
    Devices::Display->endWrite();
}