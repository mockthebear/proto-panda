#pragma once
#include "config.hpp"

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

class MatrixPanel_I2S_DMA_2 : public MatrixPanel_I2S_DMA{
    public:
        MatrixPanel_I2S_DMA_2(const HUB75_I2S_CFG &mxconfig):MatrixPanel_I2S_DMA(mxconfig){}

  
        virtual void flipDma(){ flipDMABuffer();};
        inline void updateMatrixDMABuffer_2(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue){
            updateMatrixDMABuffer(x, y, red, green, blue);
        }

        void setBrightnessExt(const uint8_t b){
            external_brightness = b;
        }

        uint8_t getBrightnessExt(){
            return external_brightness;
        }
    private:
        uint8_t external_brightness;
};

class MockDisplay : public MatrixPanel_I2S_DMA_2{
    public:
        MockDisplay(const HUB75_I2S_CFG &mxconfig):MatrixPanel_I2S_DMA_2(mxconfig){}

        bool begin(){return false;};
        void setBrightness8(const uint8_t b){};
        void clearScreen(){};
        void updateMatrixDMABuffer_2(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue){};
        void startWrite() override {};
        void endWrite() override{};
        void flipDma() override {};

        void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color){};
        void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color){};
        void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color){};
        void fillScreen(uint16_t color){};
        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){};
        void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){};
        void drawIcon(int *ico, int16_t x, int16_t y, int16_t cols, int16_t rows){};

        void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color){};
        void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color){};
        void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color){};
        void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,int16_t delta, uint16_t color){};
        void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){};
        void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color){};
        void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color){};
        void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color){};
        void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color){};
        void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg){};
        void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color){};
        void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg){};
        void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color){};
        void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h){};
        void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h){};
        void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h){};
        void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h){};
        void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h){};
        void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h){};
        void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h){};
        void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h){};
        void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size){};
        void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y){};
        void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h){};
        void getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h){};
        void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h){};
        void setTextSize(uint8_t s){};
        void setTextSize(uint8_t sx, uint8_t sy){};
        void drawPixel(int16_t x, int16_t y, uint16_t color) override {};

};

