#pragma once
#include "config.hpp"
#ifdef ENABLE_BLE
#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <queue>
#include <stack>
#include <cstring> 
#include <map> 
#include "bluetooth/characteristicshandler.hpp"
#include "bluetooth/clientcallbacks.hpp"
#include "tools/psrammap.hpp"


class DisconnectTuple{
  public:
    int id;
    int controllerId;
    std::string reason;
};


class BluetoothDeviceHandler{
  public: 
    static int idCounter;
    BluetoothDeviceHandler():m_callbacks(nullptr),m_client(nullptr),m_controllerId(0xffff),m_deviceAddress(""),m_deviceName(""),connected(false){m_id = ++idCounter;};
    ~BluetoothDeviceHandler();
    int getId(){
      return m_id;
    };
    ClientCallbacks * m_callbacks;
    NimBLEClient* m_client;
    uint32_t m_controllerId;
    std::string m_deviceAddress; 
    std::string m_deviceName; 
    bool connected;
  private:
    int m_id;
};

class AdvertisedDeviceCallbacks;

class BleServiceHandler{
  public:
    BleServiceHandler(NimBLEUUID u):uuid(u),encryptionRequired(u == NimBLEUUID((uint16_t)0x1812)),queueMutex(xSemaphoreCreateMutex()),luaOnConnectCallback(nullptr),luaOnDisconnectCallback(nullptr){};
    BleCharacteristicsHandler* AddCharacteristics(std::string uuid);

    void AddPairedDeviceAddress(std::string addr);

    void SetOnConnectCallback(LuaFunctionCallback * cb){
      luaOnConnectCallback = cb;
    }
    void SetOnDisconnectCallback(LuaFunctionCallback * cb){
      luaOnDisconnectCallback = cb;
    }

    /** Pair/bond/encrypt right after connecting. Enabled by default for the HID service (0x1812). */
    void SetEncryptionRequired(bool enable){
      encryptionRequired = enable;
    }

    void AddNameRequired(std::string namer){
      nameMap[namer] = true;
    };
    int GetClientIdFromControllerId(uint32_t id);
    void AddDevice(BluetoothDeviceHandler *dev);
    void SendMessages();
    MultiReturn<int> GetRSSI(int clientId);
    MultiReturn<std::vector<std::string>> GetServices(int clientId, bool refresh);
    MultiReturn<std::vector<std::string>> GetCharacteristicsFromOurService(int clientId);
    bool WriteToCharacteristics(std::vector<uint8_t> bytes, int clientId, std::string charName, bool reply);
    MultiReturn<std::vector<uint8_t>> ReadFromCharacteristics(int clientId, std::string charName);
    std::vector<BleCharacteristicsHandler*> getRegisteredCharacteristics();


    static MultiReturn<std::vector<std::string>> GetCharacteristicsFromService(int clientId, std::string servName, bool refresh);
    NimBLEUUID uuid;
    bool encryptionRequired;

    void NotifyDisconnect(int conId, int clientId, const char* reason);

    
  private: 
    friend AdvertisedDeviceCallbacks;
    friend BleManager;
    SemaphoreHandle_t queueMutex;
    std::stack<BluetoothDeviceHandler*> devicesToNotify;
    std::stack<DisconnectTuple> devicesToDisconnectNotify;
    PSRAMMap<std::string, BleCharacteristicsHandler*> m_characteristics;    
    PSRAMMap<std::string,bool> warnedMap;
    PSRAMVector<BluetoothDeviceHandler*> m_connectedDevices;
    PSRAMMap<std::string, bool> addrMap;
    PSRAMMap<std::string, bool> nameMap;

    LuaFunctionCallback *luaOnConnectCallback;
    LuaFunctionCallback *luaOnDisconnectCallback;
    
};


class ConnectionRequest{
    public:
        ConnectionRequest():addressType(0),address(""),name(""),handler(nullptr),deviceHandler(nullptr){};
        ConnectionRequest(std::string addressa, uint8_t addrType, std::string namea, BleServiceHandler* handlerObj, BluetoothDeviceHandler *deviceH):address(addressa),addressType(addrType),name(namea),ready(false),handler(handlerObj),deviceHandler(deviceH){if (handler != nullptr) ready = true;};
        void erase(){
            address = "";
            name = "";
            addressType = 0;
            handler = nullptr;
            ready = false;
            deviceHandler = nullptr;
        };
        bool ready;
        uint8_t addressType;
        std::string address, name;
        BleServiceHandler* handler;
        BluetoothDeviceHandler *deviceHandler;
};

#endif