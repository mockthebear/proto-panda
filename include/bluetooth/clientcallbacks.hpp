#pragma once

#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"

class BleManager;

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