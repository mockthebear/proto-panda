#pragma once

#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <queue>
#include <stack>
#include <cstring> 
#include <map> 
#include "bluetooth/characteristicshandler.hpp"
#include "bluetooth/clientcallbacks.hpp"




class BluetoothDeviceHandler{
  public: 
    static int idCounter;
    BluetoothDeviceHandler():m_device(nullptr),m_callbacks(nullptr),m_client(nullptr),m_controllerId(0xffff),connected(false){m_id = ++idCounter;};
    ~BluetoothDeviceHandler();
    int getId(){
      return m_id;
    };
    const NimBLEAdvertisedDevice* m_device;
    ClientCallbacks * m_callbacks;
    NimBLEClient* m_client;
    uint32_t m_controllerId;
    bool connected;
  private:
    int m_id;
};



class BleServiceHandler{
  public:
    BleServiceHandler(NimBLEUUID u):uuid(u),queueMutex(xSemaphoreCreateMutex()),luaOnConnectCallback(nullptr){};
    BleCharacteristicsHandler* AddCharacteristics(std::string uuid);
    void SetOnConnectCallback(LuaFunctionCallback * cb){
      luaOnConnectCallback = cb;
    }
    void AddDevice(BluetoothDeviceHandler *dev);
    void SendMessages();
    std::vector<std::string> GetServices(int clientId);
    std::vector<std::string> GetCharacteristics(int clientId);
    bool WriteToCharacteristics(std::vector<uint8_t> bytes, int clientId, std::string charName, bool reply);
    std::vector<uint8_t> ReadFromCharacteristics(int clientId, std::string charName);
    std::vector<BleCharacteristicsHandler*> getCharacteristics();
    NimBLEUUID uuid;

    
  private: 
    
    SemaphoreHandle_t queueMutex;
    std::stack<BluetoothDeviceHandler*> devicesToNotify;
    std::map<std::string, BleCharacteristicsHandler*> m_characteristics;    
    std::map<std::string,bool> warnedMap;
    std::vector<BluetoothDeviceHandler*> m_connectedDevices;

    LuaFunctionCallback *luaOnConnectCallback;
    
};


class ConnectionRequest{
    public:
        ConnectionRequest():advertisedDevice(nullptr),handler(nullptr),deviceHandler(nullptr){};
        ConnectionRequest(const NimBLEAdvertisedDevice* device, BleServiceHandler* handlerObj, BluetoothDeviceHandler *deviceH):ready(false),advertisedDevice(device),handler(handlerObj),deviceHandler(deviceH){if (handler != nullptr) ready = true;};
        void erase(){
            advertisedDevice = nullptr;
            handler = nullptr;
            ready = false;
            deviceHandler = nullptr;
        };
        bool ready;
        const NimBLEAdvertisedDevice* advertisedDevice;
        BleServiceHandler* handler;
        BluetoothDeviceHandler *deviceHandler;
};
