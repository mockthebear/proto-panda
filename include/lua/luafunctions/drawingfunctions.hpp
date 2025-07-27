#pragma once
#include <stdint.h>
#include <vector>
#include "lua/luainterface.hpp"

#ifdef ENABLE_HUB75_PANEL

void DrawPixels(std::vector<Pixel> pixels);
void DrawPixel(int16_t x, int16_t y, uint16_t color);
void DrawChar(int16_t x, int16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t size);
void DrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
int DrawLine(int16_t x, int16_t y, int16_t x1, int16_t x2, uint16_t color);
int DrawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
int DrawFillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
uint16_t color444(uint8_t r, uint8_t g, uint8_t b);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
void setPanelBrightness(uint8_t bright);
uint8_t getPanelBrightness();
bool StartPanels();
int ClearScreen();
int GetOffsetByName(std::string aliasName);
int GetFrameCountByName(std::string aliasName);
int DrawFace(int i);
void DrawCurrentFrame();
void FlipScreen();
void deleteBulkFile();
void composeBulkFile();
void setAnimation(std::vector<int> frames, int duration, int repeatTimes, bool dropAll, int externalStorageId);
void setInterruptFrames(std::vector<int> frames, int duration );
void setInterruptAnimationPin(int pin);
void setManaged(bool bn);
bool isManaged();
int getCurrentAnimationStorage();
int getCurrentFace();
void DrawPanelFaceToScreen(int x, int y);
void gentlySetPanelBrightness(uint8_t bright, uint8_t rate);
bool popPanelAnimation();
void setColorMode(int mode);
#endif