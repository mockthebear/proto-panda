#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.hpp"
#include <list>
#include <string>
#include <vector>

class OledIcon{
    public:
        OledIcon():width(0),height(0),icon(nullptr){};
        OledIcon(int w, int h, uint8_t* b):width(w),height(h),icon(b){};
        int width;
        int height;
        uint8_t *icon;
};

typedef enum InfoTypeShown_{
    SHOW_FIRST = 0,
    SHOW_FPS = 0,
    SHOW_FREE_RAM,
    SHOW_PANEL_FPS,
    SHOW_FREE_HEAP,
    SHOW_RESET,
} InfoTypeShown;

class OledScreen{
    public:
        static bool Start();
        static void DrawCircularProgress(int val, int max, const char *title);
        static void DrawProgressBar(int val, int max, const char *title);
        static void DrawWaitForPower(float volts);
        static void CriticalFail(const char *str);
        static void DrawBottomBar();
        static void Clear();
        static void SetConsoleMode(bool enable);
        static void PrintConsole(const char *str);
        static void PrintError(const char *str);
        static void DrawPanelFaceToScreen(int x, int y);
        static void DrawIcon(int x, int y, int iconId);
        static void DrawAccelerometer(int id, int posX, int posY);
        static int CreateIcon(std::vector<uint8_t> iconData, int width, int height);

        static Adafruit_SSD1306 display;
        static bool consoleMode;
        static std::list<std::string> lines;
        static uint8_t* DisplayFace;

        static std::vector<OledIcon> icons;
    private:
        static InfoTypeShown infoShown;
        static uint32_t swapTimer;
};