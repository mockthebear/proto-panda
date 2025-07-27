#include "tools/devices.hpp"
#include "tools/logger.hpp"
#include "tools/sensors.hpp"
#include "tools/oledscreen.hpp"
#include "drawing/dma_display.hpp"
#include "drawing/animation.hpp"

#include <SPI.h>
#include <Wire.h>

#if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
#include "esp32/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C2
#include "esp32c2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C6
#include "esp32c6/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32H2
#include "esp32h2/rom/rtc.h"
#else 
#error Target CONFIG_IDF_TARGET is not supported
#endif


uint64_t Devices::s_frameStart = 0;
uint64_t Devices::s_frameDuration = 0;
uint64_t Devices::s_frameAutoStart = 0;
uint64_t Devices::s_frameAutoDuration = 0;
uint32_t Devices::freeHeapBytes = 0;
uint32_t Devices::totalHeapBytes = 1;
uint32_t Devices::freePsramBytes = 0;
uint32_t Devices::totalPsramBytes = 1;
float Devices::percentagePsramFree = 1;
float Devices::percentageHeapFree = 1;
uint8_t Devices::maxBrightness = 0;


std::map<byte,bool> Devices::foundDevices;

extern Animation g_animation;

bool Devices::s_hasLidar = false;
bool Devices::s_hasServo = false;
bool Devices::s_autoCheckPower = true;

float Devices::VoltageStartThreshold = VCC_THRESHOLD_START;
float Devices::VoltageStopThreshold = VCC_THRESHOLD_HALT;

PowerMode Devices::s_powerMode = POWER_MODE_USB_5V;

bool Devices::gentlyTurnOn = false;
uint8_t Devices::targetBrigthness = 0;
uint16_t Devices::currentTargetBrigtness = 0;
uint8_t Devices::turnOnRate = 1;

#ifdef USE_LIDAR
VL53L0X Devices::lidar;
uint32_t Devices::lastLidarReadTime = 0;
uint16_t Devices::lastLidarReading = 0;
#endif

#ifdef USE_BUZZER
ToneESP32 Devices::buzzer(BUZZER_PIN, BUZZER_CHANNEL);
uint32_t Devices::m_toneDuration = 0;
#endif

#ifdef USE_SERVO
s3servo *Devices::servos[SERVO_COUNT];
#endif

#ifdef USE_INTERNAL_ACCELEROMETER
LSM6DS3 *Devices::lsm6 = nullptr;
float Devices::internalAX = 0;
float Devices::internalAY = 0;
float Devices::internalAZ = 0;
float Devices::internalGX = 0;
float Devices::internalGY = 0;
float Devices::internalGZ = 0;
float Devices::internalT = 0;
#endif

        
void Devices::Begin(){
    Wire.begin(I2C_SDA, I2C_SLC);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
}

std::vector<int> Devices::I2CScan(){ 
  byte error, address;
  int nDevices; 
  std::vector<int> foundDevicesAddr;
  Logger::Info("Running I2C Scan..."); 
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
        
        if (address<16)
            Serial.print("0");

        foundDevicesAddr.emplace_back(address);
        
        Devices::foundDevices[address] = true;
        Logger::Print("I2C device found at address %d", address);
        nDevices++;
    }
    else if (error==4)
    {
      Logger::Print("Unknown error at address ");
      Logger::Println("%d", address);
    }    
  }
  if (nDevices >= 126){
    Logger::Info("Something is really wrong with the I2C bus."); 
  }
  if (nDevices == 0){
    Logger::Info("Check the wiring please, no device has been found in the i2c bus.");
  }
  return foundDevicesAddr;
}

void Devices::SetPowerMode(PowerMode mode){
  s_powerMode = mode;
  if (s_powerMode == POWER_MODE_USB_5V){
    s_autoCheckPower = false;
    VoltageStopThreshold = 0;
    VoltageStartThreshold = 0;
  }else if (s_powerMode == POWER_MODE_USB_9V){
    s_autoCheckPower = true;
    VoltageStopThreshold = 6.5;
    VoltageStartThreshold = 8.9;
  }else if (s_powerMode == POWER_MODE_REGULATOR_PD){
    s_autoCheckPower = false;
    VoltageStopThreshold = 4.2;
    VoltageStartThreshold = 4.9;
  }else{
    s_autoCheckPower = true;
    VoltageStopThreshold = 6.5;
    VoltageStartThreshold = 8.9;
  }
}

bool Devices::AutoCheckPowerLevel(){
  return s_autoCheckPower;
}

bool Devices::CheckPowerLevel(){
  #ifdef ENABLE_HUB75_PANEL
  if (Sensors::GetAvgBatteryVoltage() < VoltageStopThreshold){
    DMADisplay::Display->clearScreen();
    DMADisplay::Display->flipDMABuffer();
    DMADisplay::Display->clearScreen();
    DMADisplay::Display->flipDMABuffer();
    return false;
  }
  #endif
  return true;
}

void Devices::SetMaxBrightness(uint8_t b){
  maxBrightness = b;
  #ifdef ENABLE_HUB75_PANEL
  DMADisplay::Display->setBrightness(b); 
  #endif
}


bool Devices::WaitForPower(uint8_t brightness){
  if (brightness == 0){
    brightness = maxBrightness;
  }
  BuzzerNoTone();
  #ifdef PIN_ENABLE_REGULATOR
  digitalWrite(PIN_ENABLE_REGULATOR, LOW);
  #endif
  int tries = 0;
  bool oldState = g_animation.isManaged();
  retry:
  tries++;

  if (tries > 10){
    OledScreen::CriticalFail("Battery is too low.");
    for (;;){}
  }
  g_animation.setManaged(false);
  //Turn off the regulator.
  #ifdef PIN_ENABLE_REGULATOR
  digitalWrite(PIN_ENABLE_REGULATOR, LOW);
  #endif
  while (Sensors::GetAvgBatteryVoltage() <= VoltageStartThreshold){
      #ifdef ENABLE_HUB75_PANEL
      DMADisplay::Display->clearScreen();
      DMADisplay::Display->flipDMABuffer();
      #endif
      OledScreen::DrawWaitForPower(Sensors::GetAvgBatteryVoltage());
      Sensors::MeasureVoltage();
  }
  //Turn on the regulator.
  #ifdef PIN_ENABLE_REGULATOR
  digitalWrite(PIN_ENABLE_REGULATOR, HIGH);
  #endif

  char buff[50];
  for (int a=1; a<brightness;a+=4){
    Sensors::FullMeasureVoltage();
    if (!Devices::CheckPowerLevel()){
        #ifdef PIN_ENABLE_REGULATOR
        digitalWrite(PIN_ENABLE_REGULATOR, LOW);
        #endif
        delay(500);
        goto retry;
    }
    sprintf(buff, "Power: %fV\nBrightness: %d/%d", Sensors::GetAvgBatteryVoltage(), a, maxBrightness );
    OledScreen::DrawProgressBar(a, maxBrightness, buff);
    uint16_t rd = Sensors::GetAvgBatteryVoltage();
    if (rd <= VoltageStopThreshold){
      digitalWrite(PIN_ENABLE_REGULATOR, LOW);
      OledScreen::DrawWaitForPower(rd);
      delay(1000);
      goto retry;
    }
    #ifdef ENABLE_HUB75_PANEL
    DMADisplay::Display->setBrightness(a); 
    DMADisplay::DrawTestScreen();
    #endif
  }
  SetMaxBrightness(brightness);
  g_animation.setManaged(oldState);
  return true;
}

void Devices::BeginFrame(){
  s_frameStart = millis();
}

void Devices::EndFrame(){
  s_frameDuration = millis()-s_frameStart;
}

void Devices::BeginAutoFrame(){
  s_frameAutoStart = millis();
  if (gentlyTurnOn){
    currentTargetBrigtness += turnOnRate;
    if (currentTargetBrigtness >= targetBrigthness){
      currentTargetBrigtness = targetBrigthness;
      gentlyTurnOn = false;
    }
    SetMaxBrightness(currentTargetBrigtness);
  }
}

void Devices::EndAutoFrame(){
  s_frameAutoDuration = millis()-s_frameAutoStart;

  if (m_toneDuration > 0 && m_toneDuration < millis()){
    m_toneDuration = 0;
    BuzzerNoTone();
  }
}

uint32_t Devices::getDeltaTime(){
  return s_frameDuration;
}

float Devices::getAutoDeltaTime(){
  return s_frameAutoDuration/1000.0f;
}

float Devices::getFps(){
  if (s_frameDuration == 0){
    return 999.9f;
  }
  return 1000.0f/s_frameDuration;
}

float Devices::getAutoFps(){
  if (s_frameDuration == 0){
    return 999.9f;
  }
  return 1000.0f/s_frameAutoDuration;
}


float Devices::getFreePsram(){
  return Devices::percentagePsramFree;
}
  

void Devices::CalculateMemmoryUsage(){

  Devices::freeHeapBytes = ESP.getFreeHeap();  
  Devices::totalHeapBytes = ESP.getHeapSize(); 
  Devices::freePsramBytes = ESP.getFreePsram(); 
  Devices::totalPsramBytes = ESP.getPsramSize(); 

  Devices::percentageHeapFree = Devices::freeHeapBytes * 100.0f / (float)Devices::totalHeapBytes;
  Devices::percentagePsramFree = freePsramBytes* 100.0f / (float)totalPsramBytes;
  Logger::Info("[Memory] %.1f%% free - %d of %d bytes free (psram: %d / %d  -> %.1f%%)", Devices::percentageHeapFree, Devices::freeHeapBytes, Devices::totalHeapBytes, totalPsramBytes, freePsramBytes,  getFreePsram()) ;
}

void Devices::DisplayResetInfo()
{
  switch (rtc_get_reset_reason(0))
  {
    case POWERON_RESET  : Logger::Info ("Reset reason: Vbat power on reset");break;
    case RTC_SW_SYS_RESET  : Logger::Info ("Reset reason: Software reset digital core");break;
    case DEEPSLEEP_RESET  : Logger::Info ("Reset reason: Deep Sleep reset digital core");break;
    case TG0WDT_SYS_RESET  : Logger::Info ("Reset reason: Timer Group0 Watch dog reset digital core");break;
    case TG1WDT_SYS_RESET  : Logger::Info ("Reset reason: Timer Group1 Watch dog reset digital core");break;
    case RTCWDT_SYS_RESET  : Logger::Info ("Reset reason: RTC Watch dog Reset digital core");break;
    case INTRUSION_RESET : Logger::Info ("Reset reason: Instrusion tested to reset CPU");break;
    case TG0WDT_CPU_RESET : Logger::Info ("Reset reason: Time Group reset CPU");break;
    case RTC_SW_CPU_RESET : Logger::Info ("Reset reason: Software reset CPU");break;
    case RTCWDT_CPU_RESET : Logger::Info ("Reset reason: RTC Watch dog Reset CPU");break;
    case RTCWDT_BROWN_OUT_RESET : Logger::Info ("Reset reason: Reset when the vdd voltage is not stable");break;
    case RTCWDT_RTC_RESET : Logger::Info ("Reset reason: RTC Watch dog reset digital core and rtc module");break;
    case TG1WDT_CPU_RESET : Logger::Info ("Time Group1 reset CPU");break;
    case SUPER_WDT_RESET : Logger::Info ("super watchdog reset digital core and rtc module");break;
    case GLITCH_RTC_RESET : Logger::Info ("glitch reset digital core and rtc module");break;
    case EFUSE_RESET : Logger::Info ("efuse reset digital core");break;
    case USB_UART_CHIP_RESET : Logger::Info ("usb uart reset digital core");break;
    case USB_JTAG_CHIP_RESET : Logger::Info ("usb jtag reset digital core");break;
    case POWER_GLITCH_RESET : Logger::Info ("power glitch reset digital core and rtc module");break;
    default : Logger::Info ("Reset reason: NO_MEAN");
  }
}

void Devices::StartAvaliableDevices(){
#ifdef USE_LIDAR
    if (Devices::foundDevices[LIDAR_ADDR]){
        if (!Devices::lidar.init()){
            Logger::Info("Failed to initialize lidar");
        }else{
            Devices::lidar.startContinuous(5);
            Devices::lastLidarReadTime = millis()+100;
            s_hasLidar = true;
            Logger::Info("Lidar initialized with %d budget", Devices::lidar.getMeasurementTimingBudget());
        }
    }
#endif
#ifdef USE_INTERNAL_ACCELEROMETER
  lsm6 = new LSM6DS3(Wire, USE_INTERNAL_ACCELEROMETER);
  if (!lsm6->begin()){
    Logger::Info("Failed to initialize internal accelerometer");
  }else{
    Logger::Info("internal accelerometer initialized");
  }
#endif
#ifdef USE_SERVO
  int pins[SERVO_COUNT] = SERVO_PINS;
  for (int i=0;i<SERVO_COUNT;i++){
    servos[i] = new s3servo();
    servos[i]->attach(pins[i], i);
  }
  s_hasServo = true;
#endif 
}

uint16_t Devices::ReadLidar(){
  if (!HasLidar()){
      return 0;
  }
  #ifdef USE_LIDAR
  if (Devices::lastLidarReadTime < millis()){
    Devices::lastLidarReading = Devices::lidar.readRangeContinuousMillimeters();
    Devices::lastLidarReadTime = millis()+20;
    //Logger::Info("Lidar reads: %d", Devices::lastLidarReading); 
  }
  return Devices::lastLidarReading;
  #else
  return 0;
  #endif
}


bool Devices::ReadInternalAcceleromter(){
  #ifdef USE_INTERNAL_ACCELEROMETER
  
    if (lsm6->accelerationAvailable()) {
      lsm6->readAcceleration(internalAX, internalAY, internalAZ);
    }
    if (lsm6->gyroscopeAvailable()) {
    }
    return true;
  #else 
    return false;
  #endif
  }
  
  bool Devices::ReadInternalTemperature(){
  #ifdef USE_INTERNAL_ACCELEROMETER
    if (lsm6->temperatureAvailable()) {
      lsm6->readTemperature(internalT);
    }
    return true;
  #else 
    return false;
  #endif
  }
  
  
  bool Devices::ReadInternalGyroscope(){
  #ifdef USE_INTERNAL_ACCELEROMETER
    if (lsm6->gyroscopeAvailable()) {
      lsm6->readGyroscope(internalGX, internalGY, internalGZ);
    }
    return true;
  #else 
    return false;
  #endif
  }
  
void Devices::ReadSensors(){
  Sensors::MeasureVoltage();
  ReadInternalAcceleromter();
  ReadInternalGyroscope();
  ReadInternalTemperature();
}


int Devices::GetSensorReading(){ //todo
  return 0;
}

bool Devices::ServoPause(int servoId){
  if (!HasServo()){
      return false;
  }
  if (servoId < 0 || servoId > SERVO_COUNT){
    return false;
  }
  #ifdef USE_SERVO
  servos[servoId]->detach(); 
  return true;
  #else
  return false;
  #endif
}
bool Devices::ServoResume(int servoId){
  if (!HasServo()){
      return false;
  }
  if (servoId < 0 || servoId > SERVO_COUNT){
    return false;
  }
  #ifdef USE_SERVO
  servos[servoId]->reattach(); 
  return true;
  #else
  return false;
  #endif
}

bool Devices::ServoMove(int servoId, float angle){
    if (!HasServo()){
        return false;
    }
    if (servoId < 0 || servoId > SERVO_COUNT){
      return false;
    }
    #ifdef USE_SERVO
    servos[servoId]->write(angle); 
    return true;
    #else
    return false;
    #endif
}

bool Devices::BuzzerTone(int tone){
  #ifdef USE_BUZZER
  buzzer.tone(tone);
  return true;
  #else
  return false;
  #endif
}

bool Devices::BuzzerToneDuration(int tone, int duration){
  #ifdef USE_BUZZER
  buzzer.tone(tone);
  m_toneDuration = millis()+duration;
  return true;
  #else
  return false;
  #endif
}

bool Devices::BuzzerNoTone(){
  #ifdef USE_BUZZER
  buzzer.noTone();
  return true;
  #else
  return false;
  #endif
}

