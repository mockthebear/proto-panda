#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ArduinoJson.h>


class HardwareConfig{
    public:
        static bool LoadConfigs();
        static bool StartDmaDisplay();
        
    private:
        static HUB75_I2S_CFG panelConfig;
        static void loadHub75AndStart(JsonObject doc);
        static void loadServosAndStart(JsonObject doc);
        static void loadDefaults();
        static int checkInvalidPin(int pin);

};


