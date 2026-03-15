#pragma once
#include <stdint.h>
#include <vector>
#include "lua/luainterface.hpp"
#include "drawing/rendering/model.hpp"

#ifdef ENABLE_HUB75_PANEL

void DrawPixels(std::vector<Pixel> pixels);
void DrawPixel(int16_t x, int16_t y, uint16_t color);
void DrawChar(int16_t x, int16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t size);
void DrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void DrawLine(int16_t x, int16_t y, int16_t x1, int16_t x2, uint16_t color);
void DrawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void DrawFillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void DrawFillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
uint16_t color444(uint8_t r, uint8_t g, uint8_t b);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
void setPanelBrightness(uint8_t bright);
uint8_t getPanelBrightness();

int ClearScreen();
int GetOffsetByName(std::string aliasName);
int GetFrameCountByName(std::string aliasName);
int DrawFace(int i);
void DrawCurrentFrame();

void deleteBulkFile();
void composeBulkFile();



void DrawPanelFaceToScreen(int x, int y);
void gentlySetPanelBrightness(uint8_t bright, uint8_t rate);


#endif