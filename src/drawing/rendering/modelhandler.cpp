#include "drawing/rendering/modelhandler.hpp"
#include "tools/devices.hpp"
#include "tools/oledscreen.hpp"

void ModelHandler::RenderModels(std::vector<Model*> mdls, uint8_t *bitmap){
    for (auto& model : mdls) {
        if (model->triangleCount == 0) continue;
        
        for (int i = model->triangleCount - 1; i >= 0; i--) {
            model->RasterTriangleWithBitmap(this, i, bitmap);
        }
    }
}


void ModelHandler::RenderScene(std::vector<Model*> mdls){
    Devices::Display->startWrite();
    memset(pixelBitmap, 0, sizeof(pixelBitmap));
    uint8_t r, g, b;
    Devices::Display->color565to888(0, r, g, b);
    int byteIdOled = 0;
    uint8_t* nextId = OledScreen::DisplayFace[(OledScreen::screenFlipId+1)%2];
    for (float y = 0; y < PANEL_HEIGHT; y++) {
        for (float x = 0; x < PANEL_WIDTH; x++) {
            Devices::Display->updateMatrixDMABuffer_2(x, y, r, g, b);
            Devices::Display->updateMatrixDMABuffer_2((PANEL_WIDTH) + x, y, r, g, b);
            nextId[byteIdOled] = 0;
            byteIdOled++;
        }
    }
    RenderModels(mdls, nextId);
    OledScreen::screenFlipId++;
    Devices::Display->endWrite();
}