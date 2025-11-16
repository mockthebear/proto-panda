#pragma once

#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <queue>
#include <cstring> 
#include <map> 
#include "bluetooth/characteristicshandler.hpp"
#include "bluetooth/clientcallbacks.hpp"






class BleServiceHandler{
  public:
    BleServiceHandler(NimBLEUUID u):uuid(u),queueMutex(xSemaphoreCreateMutex()){
    }
    void AddCharacteristics(NimBLEUUID charId);
    void AddCharacteristics_TMP(NimBLEUUID charId);
    void AddMessage(NimBLEUUID &charId, uint8_t* pData, size_t length, bool isNotify);
    void SendMessages();
    std::vector<BleCharacteristicsHandler*> getCharacteristics();
    NimBLEUUID uuid;
  private: 
    SemaphoreHandle_t queueMutex;
    std::map<std::string, BleCharacteristicsHandler*> m_characteristics;    
};


class ConnectionRequest{
    public:
        ConnectionRequest():advertisedDevice(nullptr),handler(nullptr){};
        ConnectionRequest(NimBLEAdvertisedDevice* device, BleServiceHandler* handlerObj):advertisedDevice(device),handler(handlerObj){};
        void erase(){
            advertisedDevice = nullptr;
            handler = nullptr;
        };
        NimBLEAdvertisedDevice* advertisedDevice;
        BleServiceHandler* handler;
};

class BluetoothDeviceHandler{
    public: 
        BluetoothDeviceHandler():m_device(nullptr),m_callbacks(nullptr),m_client(nullptr),m_controllerId(0xffff),connected(false){};
        ~BluetoothDeviceHandler();
        NimBLEAdvertisedDevice* m_device;
        ClientCallbacks * m_callbacks;
        NimBLEClient* m_client;
        uint32_t m_controllerId;
        bool connected;
};
