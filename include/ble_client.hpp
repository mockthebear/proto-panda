#pragma once
#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <map>
#include <vector>
#include <stack>
#include <queue>
#include <cstring> 

#define BUTTON_RELEASED 0
#define BUTTON_JUST_PRESSED 1
#define BUTTON_PRESSED 2
#define BUTTON_JUST_RELEASED 3

class BleServiceHandler{
  public:
    BleServiceHandler(NimBLEUUID u):uuid(u){
      queueMutex = xSemaphoreCreateMutex();
    }
    void AddMessage(uint8_t* pData, size_t length, bool isNotify);
  private: 
    SemaphoreHandle_t queueMutex;
    std::queue<BleMessage> dataQueue;
    NimBLEUUID uuid;
};


class BleMessage{
  public:
    BleMessage(uint8_t* pData, size_t length):message(pData, pData + length){}
    std::vector<uint8_t> message;
};

class BleManager;

class BleSensorData{
  public:
    BleSensorData():z(0),x(0),y(0),az(0),ax(0),ay(0),temp(0),controllerId(0){
      for (int i=0;i<MAX_BLE_BUTTONS;i++){
        buttons[i] = 0;
      }
    }
    int16_t z,x,y;
    int16_t az,ax,ay;
    int16_t temp;
    uint8_t buttons[MAX_BLE_BUTTONS+2];
    uint8_t controllerId;
};

class BleSensorHandlerData : public BleSensorData{
  public:
    BleSensorHandlerData():BleSensorData(){}
  
    void copy(BleSensorData* aux){
      this->x = aux->x;
      this->y = aux->y;
      this->z = aux->z;
      this->ax = aux->ax;
      this->ay = aux->ay;
      this->az = aux->az;
      this->temp = aux->temp;
      this->controllerId = aux->buttons[0];
      for (int i = 1; i < MAX_BLE_BUTTONS+1; ++i) {
          this->buttons[i-1] = aux->buttons[i];
      }
    }

    void setLastUpdate(){
      currentUpdate = millis()-previousUpdate;
      previousUpdate = millis();
    }

    void updateButtons();

    uint32_t last_inputButtonsStatus[MAX_BLE_BUTTONS];  
    uint32_t real_inputButtonsStatus[MAX_BLE_BUTTONS];  
    uint32_t previousUpdate; 
    uint32_t currentUpdate; 
};

class AccelerometerData{
  public:
    int x,y,z;
};

class ClientCallbacks : public NimBLEClientCallbacks {
    
    void onConnect(NimBLEClient* pClient);
    void onDisconnect(NimBLEClient* pClient);
    bool onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params);
    uint32_t onPassKeyRequest();
    bool onConfirmPIN(uint32_t pass_key);
    void onAuthenticationComplete(ble_gap_conn_desc* desc);
  public:
    ~ClientCallbacks() {};
    ClientCallbacks():bleObj(nullptr){};
    BleManager *bleObj;


};

class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice);
  public:
    BleManager *bleObj;
};

class ConnectTuple{
  public:
    ConnectTuple(NimBLEAdvertisedDevice* device, NimBLEUUID service, NimBLEUUID streamCharacteristics, NimBLEUUID idCharacteristics):m_device(device),m_service(service),m_streamCharacteristic(streamCharacteristics),m_idCharacteristic(idCharacteristics),callbacks(nullptr),m_client(nullptr),m_controllerId(0xffff),connected(true){};
    NimBLEAdvertisedDevice* m_device;
    NimBLEUUID m_service;
    NimBLEUUID m_streamCharacteristic;
    NimBLEUUID m_idCharacteristic;
    ClientCallbacks * callbacks;
    NimBLEClient* m_client;
    uint32_t m_controllerId;
    bool connected;

    
};


class BleManager{
  public:
    BleManager():clientCount(0), maxClients(1),lastScanClearTime(0),m_started(false),m_canScan(false),m_logDiscoveredClients(false),nextId(0),m_mutex(xSemaphoreCreateMutex()){}
    bool begin();
    void update();
    void updateButtons();
    void beginScanning();

    int acceptTypes(std::string service, std::string characteristic, std::string characteristicId);
    void setMaximumControls(int n){maxClients = n;};

    int getConnectedClients(){
      return clients.size();
    }

    bool isElementIdConnected(int id);

    bool hasChangedClients();
    void setLogDiscoveredClients(bool log){
      m_logDiscoveredClients = log;
    }

    bool canLogDiscoveredClients(){
      return m_logDiscoveredClients;
    }


    static BleSensorHandlerData remoteData[MAX_BLE_CLIENTS];


    std::vector<std::tuple<NimBLEUUID,NimBLEUUID,NimBLEUUID>> GetAcceptedUUIDS(){
      return m_acceptedUUIDs;
    }

    void AddMessageToQueue(NimBLEUUID &&svcUUID, NimBLEUUID &&charUUID, uint8_t* pData, size_t length, bool isNotify);
    static BleManager* Get();
  private:
    std::map<NimBLEUUID, BleServiceHandler*> handlers;
    std::map<std::string,ConnectTuple*> clients;
    bool connectToServer(ConnectTuple *tpl);
    uint16_t clientCount;
  
    uint32_t  maxClients, lastScanClearTime;
    bool m_started, m_canScan, m_logDiscoveredClients;
    std::stack<uint8_t> availableIds;
    uint8_t nextId;

    std::vector<std::tuple<NimBLEUUID,NimBLEUUID,NimBLEUUID>> m_acceptedUUIDs;
    SemaphoreHandle_t m_mutex;
    static BleManager *m_myself;
    friend class ClientCallbacks;
};

extern BleManager g_remoteControls;
