#pragma once

#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <queue>
#include <cstring> 
#include "lua/LuaWrapper.h"

class BleMessage{
  public:
    BleMessage(uint8_t* pData, size_t length):message(pData, pData + length){}
    std::vector<uint8_t> message;
};

typedef std::function<void(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)> notify_callback;

class BleCharacteristicsHandler{
  public:
    BleCharacteristicsHandler():uuid(uint32_t(0)),required(false),notify(false),luaCallback(nullptr),queueMutex(xSemaphoreCreateMutex()){createLambda();};
    BleCharacteristicsHandler(NimBLEUUID u, bool req=true, bool notif=true):uuid(u),required(req),notify(notif),luaCallback(nullptr),queueMutex(xSemaphoreCreateMutex()){
      createLambda();
    }
    void SetSubscribeCallback(LuaFunctionCallback *lcb){
      luaCallback = lcb;
    };
    
    void AddMessage(uint8_t* pData, size_t length, bool isNotify);
    void SendMessages();
    notify_callback getLambda(){
        return notificationLambda;
    }

    NimBLEUUID uuid;
    bool required;
    bool notify;
    LuaFunctionCallback *luaCallback;
  private: 
    void createLambda();
    SemaphoreHandle_t queueMutex;
    std::queue<BleMessage> dataQueue;
    notify_callback notificationLambda;
};