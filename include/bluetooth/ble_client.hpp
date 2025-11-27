#pragma once
#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"
#include <map>
#include <vector>
#include <stack>
#include <cstring> 
#include "bluetooth/servicehandler.hpp"

#define BUTTON_RELEASED 0
#define BUTTON_JUST_PRESSED 1
#define BUTTON_PRESSED 2
#define BUTTON_JUST_RELEASED 3

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



class AdvertisedDeviceCallbacks: public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;
  void onScanEnd(const NimBLEScanResults& results, int reason) override ;
  public:
    BleManager *bleObj;
};

/*


*/


class BleManager{
  public:
    BleManager():clientCount(0), maxClients(2),lastScanClearTime(0),m_started(false),m_canScan(false),m_logDiscoveredClients(false),isScanning(false),nextId(0),m_mutex(xSemaphoreCreateMutex()){}
    bool begin();
    bool beginRadio();
    void update();
    void sendUpdatesToLua();
    void updateButtons();
    void beginScanning();

    
    void setMaximumControls(int n){maxClients = n;};

    int getConnectedClientsCount(){
      return 0; handlers.size();
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
    void AddAcceptedService(std::string name, BleServiceHandler* obj);

    std::map<std::string, BleServiceHandler*> &GetAcceptedServices(){
      return handlers;
    }

    void setScanningMode(bool scan);

    static BleManager* Get();
  private:
    std::vector<std::tuple<std::string,BleServiceHandler*>> handlersAsync; //Handlers are stored by their UUID
    std::map<std::string, BleServiceHandler*> handlers; //Handlers are stored by their UUID
    std::map<std::string, BluetoothDeviceHandler*> clients; //Clients are stored by their address

    bool connectToServer();
    bool connectToServerOLD();
    uint16_t clientCount;
  
    uint32_t  maxClients, lastScanClearTime;
    bool m_started, m_canScan, m_logDiscoveredClients, isScanning;
    std::stack<uint8_t> availableIds;
    uint8_t nextId;


    SemaphoreHandle_t m_mutex;
    static BleManager *m_myself;

    ConnectionRequest toConnect;

    friend class AdvertisedDeviceCallbacks;
    friend class ClientCallbacks;
};

extern BleManager g_remoteControls;
