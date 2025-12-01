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
