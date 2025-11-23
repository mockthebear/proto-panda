#pragma once

#include <NimBLEDevice.h>
#include "Arduino.h"
#include "config.hpp"

class BleManager;


class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) override;

    void onDisconnect(NimBLEClient* pClient, int reason) override ;

    /********************* Security handled here *********************/
    void onPassKeyEntry(NimBLEConnInfo& connInfo) override;

    void onConfirmPasskey(NimBLEConnInfo& connInfo, uint32_t pass_key) override;

    /** Pairing process complete, we can check the results in connInfo */
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
};


