#pragma once

#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <queue>
#include <cstring> 
#include "lua/LuaWrapper.h"

class BleMessage{
  public:
    BleMessage(int id, uint8_t* pData, size_t length):message(pData, pData + length),m_id(id){}
    std::vector<uint8_t> message;
    int m_id;
};

typedef std::function<void(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)> notify_callback;

class BleCharacteristicsHandler{
  public:
    BleCharacteristicsHandler():uuid(uint32_t(0)),required(false),notify(false),luaCallback(nullptr),queueMutex(xSemaphoreCreateMutex()){};
    BleCharacteristicsHandler(NimBLEUUID u, bool req=true, bool notif=true):uuid(u),required(req),notify(notif),luaCallback(nullptr),queueMutex(xSemaphoreCreateMutex()){
    }
    void SetSubscribeCallback(LuaFunctionCallback *lcb){
      luaCallback = lcb;
    };
    
    void AddMessage(int id, uint8_t* pData, size_t length, bool isNotify);
    void SendMessages();
    notify_callback getLambda(int id){
        return [this, id](NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
          this->AddMessage(id, pData, length, isNotify);
        };
    }

    NimBLEUUID uuid;
    bool required;
    bool notify;
    LuaFunctionCallback *luaCallback;
  private: 
    SemaphoreHandle_t queueMutex;
    std::queue<BleMessage> dataQueue;
    notify_callback notificationLambda;
};