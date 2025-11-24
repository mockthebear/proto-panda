#pragma once

#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <queue>
#include <cstring> 


class BleMessage{
  public:
    BleMessage(uint8_t* pData, size_t length):message(pData, pData + length){}
    std::vector<uint8_t> message;
};

class BleCharacteristicsHandler{
  public:
    BleCharacteristicsHandler():uuid(uint32_t(0)),required(false),notify(false),queueMutex(xSemaphoreCreateMutex()){};
    BleCharacteristicsHandler(NimBLEUUID u, bool req=true, bool notif=true, bool tpsendp=false):uuid(u),required(req),notify(notif),temporary_send_legacy_packet(tpsendp),queueMutex(xSemaphoreCreateMutex()){
    }
    void SetSubscribeCallback(int n){
      Serial.printf("chamou o set callback, que gay: %d\n", n);
    };
    void AddMessage(uint8_t* pData, size_t length, bool isNotify);
    void SendMessages();
    NimBLEUUID uuid;
    bool required;
    bool notify;
    bool temporary_send_legacy_packet;
  private: 
    
    SemaphoreHandle_t queueMutex;
    std::queue<BleMessage> dataQueue;
      
};