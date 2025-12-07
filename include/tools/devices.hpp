#pragma once

#include "config.hpp"
#include <Arduino.h>
#ifdef USE_LIDAR
    #include <VL53L0X.h>
#endif
#ifdef USE_INTERNAL_ACCELEROMETER
    #include "tools/LSM6DS3.hpp"
#endif

#include "tools/s3servo.hpp"



#include <vector>
#include <map>
/*

    This class is responsible for all onboard sensors.

*/

enum PowerMode
{
    POWER_MODE_USB_5V=0,
    POWER_MODE_USB_9V=1,
    POWER_MODE_BATTERY=2,
    POWER_MODE_REGULATOR_PD=3,
};

class Devices{
    public:
        static void Begin();
        static std::vector<int> I2CScan();
        static void CalculateMemmoryUsage();
        static void CalculateMemmoryUsageDifference(const std::string msg = "");
        static void StartAvaliableDevices();
        static bool CheckPowerLevel();
        static bool AutoCheckPowerLevel();
        static void SetPowerMode(PowerMode mode);
        static void SetAutoCheckPowerLevel(bool b){
            s_autoCheckPower = b;
        };
        static bool WaitForPower(uint8_t brightness);
        static void UpdateFrameSpeed(uint32_t mils);
        static void DisplayResetInfo();

        static bool HasLidar(){
            return s_hasLidar;
        }
        static bool HasServo(){
            return s_hasServo;
        }
        static uint16_t ReadLidar();
        static void BeginFrame();
        static void EndFrame();
        static void BeginAutoFrame();
        static void EndAutoFrame();
        static uint32_t getDeltaTime();
        static float getAutoDeltaTime();

        static void ReadSensors();

        static float getFps();
        static float getAutoFps();

        static void SetMaxBrightness(uint8_t b);
        static uint8_t GetMaxBrightness(){
            return maxBrightness;
        }

       
        static int GetSensorReading();

        static void SetVoltageStartThreshold(float v){
            VoltageStartThreshold = v;
        }
        static void SetVoltageStopThreshold(float v){
            VoltageStopThreshold = v;
        }

        static bool ReadInternalAcceleromter();
        static bool ReadInternalGyroscope();
        static bool ReadInternalTemperature();

        static void SetGentlyBrightness(uint8_t bright, uint8_t rate = 1, uint8_t startAmount=0){
            if (maxBrightness == bright){
                SetMaxBrightness(startAmount);
                return;
            }
            SetMaxBrightness(startAmount);
            gentlyTurnOn = true;
            targetBrigthness = maxBrightness = bright;
            turnOnRate = turnOnRate;
            currentTargetBrigtness = 0;
            if (turnOnRate == 0){
                turnOnRate = 1;
            }
        };
        
        static bool ServoMove(int servoId, float angle);
        static bool ServoPause(int servoId);
        static bool ServoResume(int servoId);
        static bool BuzzerToneDuration(int tone, int duration);
        static bool BuzzerTone(int tone);
        static bool BuzzerNoTone();

        static float getFreePsram();
        static float getFreeHeap(){
            return percentageHeapFree;
        }

        static float getInternalAccelerometerX(){
            return internalAX;
        };
        static float getInternalAccelerometerY(){
            return internalAY;
        };
        static float getInternalAccelerometerZ(){
            return internalAZ;
        };
        static float getInternalTemperature(){
            return internalT;
        };

        static float getInternalGyroscopeX(){
            return internalGX;
        };
        static float getInternalGyroscopeY(){
            return internalGY;
        };
        static float getInternalGyroscopeZ(){
            return internalGZ;
        };
        
    private:
        static uint8_t maxBrightness;
        static uint32_t freeHeapBytes;
        static uint32_t totalHeapBytes;
        static uint32_t freePsramBytes;
        static uint32_t totalPsramBytes;    
        static float percentageHeapFree;
        static float percentagePsramFree;

        static std::map<byte,bool> foundDevices;

        static uint64_t s_frameStart;
        static uint64_t s_frameAutoStart;
        static uint64_t s_frameDuration;
        static uint64_t s_frameAutoDuration;    
        static bool s_hasLidar;
        static bool s_autoCheckPower;
        static bool s_hasServo;
        static PowerMode s_powerMode;

        static float VoltageStartThreshold;
        static float VoltageStopThreshold;

        static bool gentlyTurnOn;
        static uint8_t targetBrigthness;
        static uint16_t currentTargetBrigtness;
        static uint8_t turnOnRate;

#ifdef USE_INTERNAL_ACCELEROMETER
        static LSM6DS3 *lsm6;
#endif
        static float internalAX, internalAY, internalAZ;
        static float internalGX, internalGY, internalGZ;
        static float internalT;

#ifdef USE_BUZZER
        static ToneESP32 buzzer;
        static uint32_t m_toneDuration;
#endif

#ifdef USE_SERVO
        static s3servo *servos[SERVO_COUNT];
#endif


#ifdef USE_LIDAR
        static VL53L0X lidar;
        static uint32_t lastLidarReadTime;
        static uint16_t lastLidarReading;
#endif
};