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


class OledScreen{
    public:
        static bool Start();
        static void DrawCircularProgress(int val, int max, const char *title);
        static void DrawProgressBar(int val, int max, const char *title);
        static void DrawWaitForPower(float volts);
        static void CriticalFail(const char *str);
        static void Clear();
        static void SetConsoleMode(bool enable);
        static void PrintConsole(const char *str);
        static void PrintError(const char *str);
        static void DrawPanelFaceToScreen(int x, int y);
        static void DrawIcon(int x, int y, int iconId);
        static int CreateIcon(std::vector<uint8_t> iconData, int width, int height);

        static Adafruit_SSD1306 display;
        static bool consoleMode;
        static std::list<std::string> lines;
        static uint8_t* DisplayFace;

        static std::vector<OledIcon> icons;
    private:
        static uint32_t swapTimer;
};