#include "tools/sensors.hpp"

#ifdef USE_PIN_BATTERY_IN
uint16_t Sensors::voltages[MEASUREMENTS];
uint16_t Sensors::pointer = 0;
#endif

void Sensors::Start(){
    FullMeasureVoltage();   
}

uint16_t Sensors::MeasureVoltage(){
    #ifdef USE_PIN_BATTERY_IN
        uint16_t reading = analogRead(PIN_USB_BATTERY_IN);
        Sensors::voltages[pointer++] = reading;
        if (pointer == (MEASUREMENTS-1)){
            pointer = 0;
        }
        return reading;
    #else
        return 0;
    #endif
}

void Sensors::FullMeasureVoltage(){
    #ifdef USE_PIN_BATTERY_IN
    for (int i=0;i<MEASUREMENTS;i++){
        Sensors::MeasureVoltage();
        delay(2);
    }
    #endif
}

float Sensors::GetBatteryVoltage(){
    #ifdef USE_PIN_BATTERY_IN
        return Sensors::calculateVoltage(Sensors::MeasureVoltage());
    #else 
        return 0.0f;
    #endif
}

float Sensors::GetAvgBatteryVoltage(){
    #ifdef USE_PIN_BATTERY_IN
    uint32_t val = 0;
    for (int i=0;i<MEASUREMENTS;i++){
        val += Sensors::voltages[i];
    }
    return Sensors::calculateVoltage(val/(float)MEASUREMENTS);
    #else
    return 0.0f;
    #endif
}

float Sensors::calculateVoltage(uint16_t val){
    return val * (V_REF / 4095.0) * ((RESISTOR_DIVIDER_R8 + RESISTOR_DIVIDER_R9) / RESISTOR_DIVIDER_R9);
}