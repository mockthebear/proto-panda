#include "drawing/rendering/modelhandler.hpp"
#include "tools/devices.hpp"

void ModelHandler::RenderModels(){
    for (auto& model : models) {
        if (model->triangleCount == 0) continue;
        
        for (int i = model->triangleCount - 1; i >= 0; i--) {
            model->RasterTriangle(this, i);
        }
    }
}


void ModelHandler::RenderScene(){
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