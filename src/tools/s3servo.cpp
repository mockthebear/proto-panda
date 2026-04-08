/*
   s3servo.cpp - Library for control servo motor on esp32s3.
   Created by HA.S, October 10, 2022.
   Released into the public domain.
   mit라이선스라고하고싶은데 arduino-esp32 라이선스 따라가야겠지? 아직 라이선스 잘모르니 그냥 arduino-esp32 이거 사용"했으니 이거 따름
*/
#include "tools/s3servo.hpp"

#if defined(ESP_IDF_VERSION_MAJOR)
  #if ESP_IDF_VERSION_MAJOR >= 5
    #define USE_NEW_LEDC_API 1  // Core 3.x with IDF 5.x
  #else
    #define USE_NEW_LEDC_API 0  // Core 2.x with IDF 4.x
  #endif
#else
  // If ESP_IDF_VERSION_MAJOR is not defined, assume old API
  #define USE_NEW_LEDC_API 0
#endif

ToneESP32::ToneESP32(int pin, int channel) {	
  this->pin = pin;
  this->channel = channel;
  
  #if USE_NEW_LEDC_API
    // New API (Core 3.x): attach with default frequency
    // Don't set frequency yet - will be set in tone()
    #ifdef PWM_Res
      ledcAttach(pin, 5000, PWM_Res);
    #else
      ledcAttach(pin, 5000, 8);  // Default resolution 8-bit
    #endif
  #else
    // Old API (Core 2.x): just attach pin, frequency will be set in tone()
    ledcAttachPin(pin, channel);
  #endif
}

void ToneESP32::tone(int note) {
  #if USE_NEW_LEDC_API
    // New API (Core 3.x): detach and reattach with new frequency
    ledcDetach(pin);
    #ifdef PWM_Res
      ledcAttach(pin, note, PWM_Res);
    #else
      ledcAttach(pin, note, 8);
    #endif
    ledcWrite(pin, 127);
  #else
    // Old API (Core 2.x): use channel-based setup
    #ifdef PWM_Res
      ledcSetup(channel, note, PWM_Res);
    #else
      ledcSetup(channel, note, 8);
    #endif
    ledcWrite(channel, 127);
  #endif
}

void ToneESP32::noTone() {
  #if USE_NEW_LEDC_API
    ledcWrite(pin, 0);
  #else
    ledcWrite(channel, 0);
  #endif
}

s3servo::s3servo() {
}

s3servo::~s3servo() {
    detach();
}

void s3servo::detach() {
  #if USE_NEW_LEDC_API
    ledcDetach(_pin);
  #else
    ledcDetachPin(_pin);
  #endif
}

void s3servo::reattach(){
  #if USE_NEW_LEDC_API
    ledcAttach(_pin, 50, MAX_BIT_NUM);
  #else
    ledcAttachPin(_pin, _channel);
    ledcSetup(_channel, 50, MAX_BIT_NUM);
  #endif
}

void s3servo::_setAngleRange(int min, int max){
    _minAngle = min;
    _maxAngle = max;
}

void s3servo::_setPulseRange(int min, int max){
    _minPulseWidth = min;
    _maxPulseWidth = max;
}

int8_t s3servo::attach(int pin, int channel, int min_angle, int max_angle, int min_pulse, int max_pulse)
{
    if(CHANNEL_MAX_NUM < channel || channel < 0){
        return -1;
    }
    _pin = pin; 
    _channel = channel;
    _setAngleRange(min_angle, max_angle);
    _setPulseRange(min_pulse, max_pulse);
    
    #if USE_NEW_LEDC_API
      // New API (Core 3.x): attach directly
      ledcAttach(pin, 50, MAX_BIT_NUM);
    #else
      // Old API (Core 2.x): setup and attach separately
      ledcSetup(channel, 50, MAX_BIT_NUM);
      ledcAttachPin(pin, channel);
    #endif
    
    return 0;
}

float s3servo::mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void s3servo::write(float angle) {
    int duty = (int)mapf(angle, _minAngle, _maxAngle, _minPulseWidth, _maxPulseWidth);
    if (duty < _minPulseWidth){
        duty = _minPulseWidth;
    }
    if (duty > _maxPulseWidth){
        duty = _maxPulseWidth;
    }
    
    #if USE_NEW_LEDC_API
      ledcWrite(_pin, duty);
    #else
      ledcWrite(_channel, duty);
    #endif
}

void s3servo::writeDuty(int duty) {
    #if USE_NEW_LEDC_API
      ledcWrite(_pin, duty);
    #else
      ledcWrite(_channel, duty);
    #endif
}