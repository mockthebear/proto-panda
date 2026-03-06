#pragma once

#include <config.hpp>
#include <Arduino.h>

/*

    This class is responsible for all onboard sensors.

*/

#define MEASUREMENTS 32

class Sensors{
    public:
        static void Start();
        static float GetBatteryVoltage();
        static float GetAvgBatteryVoltage();
        static uint16_t MeasureVoltage();
        static void FullMeasureVoltage();
    private:
        static float calculateVoltage(uint16_t val);
        #ifdef USE_PIN_BATTERY_IN
        static uint16_t voltages[MEASUREMENTS];
        static uint16_t pointer;
        #endif
};