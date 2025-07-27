#include "lua/luafunctions/drawingfunctions.hpp"
#include "drawing/dma_display.hpp"
#include "drawing/framerepository.hpp"
#include "drawing/animation.hpp"
#include "tools/oledscreen.hpp"
#include "tools/devices.hpp"
#include <FFat.h>

#ifdef ENABLE_HUB75_PANEL

extern FrameRepository g_frameRepo;
extern Animation g_animation;

void DrawPixels(std::vector<Pixel> pixels)
{
  DMADisplay::Display->startWrite();
  for (auto &it : pixels ){
    DMADisplay::Display->updateMatrixDMABuffer_2(it.x, it.y, it.r, it.g, it.b);
  }
  DMADisplay::Display->endWrite();
}


void DrawPixel(int16_t x, int16_t y, uint16_t color)
{
  DMADisplay::Display->drawPixel(x, y, color);
}

void DrawChar(int16_t x, int16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t size)
{
  DMADisplay::Display->drawChar(x, y, c, color, bg, size);
}


void DrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  DMADisplay::Display->fillRect(x, y, w, h, color);
}

void DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  DMADisplay::Display->drawRect(x, y, w, h, color);
}

int DrawLine(int16_t x, int16_t y, int16_t x1, int16_t x2, uint16_t color)
{
  DMADisplay::Display->drawLine(x, y, x1, x2, color);
  return 0;
}

int DrawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  DMADisplay::Display->drawCircle(x, y, r, color);
  return 0;
}

int DrawFillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  DMADisplay::Display->fillCircle(x, y, r, color);
  return 0;
}

uint16_t color444(uint8_t r, uint8_t g, uint8_t b)
{
  return DMADisplay::Display->color444(r, g, b);
}

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
  return DMADisplay::Display->color565(r,g,b);
}

void setPanelBrightness(uint8_t bright)
{
  DMADisplay::Display->setBrightness(bright);
  DMADisplay::Display->setBrightnessExt(bright);
  return;
}

uint8_t getPanelBrightness()
{
  return DMADisplay::Display->getBrightnessExt();
}


bool StartPanels()
{
  return g_frameRepo.Begin() && DMADisplay::Start(4);
}


int ClearScreen()
{
  DMADisplay::Display->clearScreen();
  return 0;
}



int GetOffsetByName(std::string aliasName)
{
  return g_frameRepo.getOffsetByName(aliasName);
}

int GetFrameCountByName(std::string aliasName)
{
  return g_frameRepo.getFrameCountByName(aliasName);
}

int DrawFace(int i)
{
  g_animation.DrawFrame(g_frameRepo.takeFile(), i);
  g_frameRepo.freeFile();
  return 0;
}
void DrawCurrentFrame()
{
  g_animation.DrawCurrentFrame(g_frameRepo.takeFile());
  g_frameRepo.freeFile();
}

void FlipScreen()
{
  g_animation.MakeFlip();
}
void deleteBulkFile(){
  FFat.remove("/frames.bulk");
}

void composeBulkFile()
{
  g_frameRepo.composeBulkFile();
}

void setAnimation(std::vector<int> frames, int duration, int repeatTimes, bool dropAll, int externalStorageId)
{
  g_animation.SetAnimation(duration, frames, repeatTimes, dropAll, externalStorageId);
}

void setInterruptFrames(std::vector<int> frames, int duration )
{
  g_animation.SetInterruptAnimation(duration, frames);
}
void setInterruptAnimationPin(int pin)
{
  if (pin > 0){
    pinMode(pin, INPUT);
  }
  g_animation.SetInterruptPin(pin);
}


void setManaged(bool bn)
{
  g_animation.setManaged(bn);
}
bool isManaged()
{
  return g_animation.isManaged();
}

int getCurrentAnimationStorage()
{
  return g_animation.getCurrentAnimationStorage();
}
int getCurrentFace()
{
  return g_animation.getCurrentFace();
}

void DrawPanelFaceToScreen(int x, int y)
{
  OledScreen::DrawPanelFaceToScreen(x, y);
  return;
}


void gentlySetPanelBrightness(uint8_t bright, uint8_t rate)
{
  Devices::SetGentlyBrightness(bright, rate);
  return;
}


bool popPanelAnimation()
{
  return g_animation.PopAnimation();
}

void setColorMode(int mode){
  g_animation.setColorMode((ColorMode)mode);
}


#endif