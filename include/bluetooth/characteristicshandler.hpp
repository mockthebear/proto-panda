#pragma once

#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <queue>
#include <cstring> 
#include "lua/LuaWrapper.h"

class BleMessage{
  public:
    BleMessage(int cliId, int  id, uint8_t* pData, size_t length):m_CliId(cliId),m_id(id),message(pData, pData + length){}
    int m_CliId;
    int m_id;
    std::vector<uint8_t> message;
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
    
    void AddMessage(int cliID, int id, uint8_t* pData, size_t length, bool isNotify);
    void SendMessages();
    notify_callback getLambda(int id, int cliID){
        return [this, id, cliID](NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
          this->AddMessage(cliID, id, pData, length, isNotify);
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