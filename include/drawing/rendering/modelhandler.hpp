#pragma once
#include "drawing/rendering/model.hpp"


class ModelHandler {
    public:
        __attribute__((aligned(32)))
        uint8_t pixelBitmap[PANEL_HEIGHT * (PANEL_WIDTH/8)];

        void RenderScene();
        void RenderModels();

        int addModel(Model *m){
            models.emplace_back(m);
            return models.size() -1;
        };
                    
        inline bool IRAM_ATTR MarkPixel(int x, int y) {
            uint8_t* bytePtr = &pixelBitmap[(y * (PANEL_WIDTH/8)) + (x >> 3)];
            uint8_t mask = 1 << (x & 7);
            if (*bytePtr & mask) return true;
            *bytePtr |= mask;
            return false;
        }

        std::vector<Model*> models;
};
