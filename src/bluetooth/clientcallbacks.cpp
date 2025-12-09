#include "bluetooth/clientcallbacks.hpp"
#include "bluetooth/ble_client.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

void ClientCallbacks::onConnect(NimBLEClient* pClient) { 
  Serial.printf("Connected to HID device\n"); 
  Logger::Info("[BLE] Device Connected");
  //xSemaphoreTake(g_remoteControls.m_mutex, portMAX_DELAY);
  //auto aux = g_remoteControls.clients[pClient->getPeerAddress().toString()];
  //Logger::Info("[BLE] aaaaa");
  //xSemaphoreGive(g_remoteControls.m_mutex);
  /*if (aux != nullptr){
    Logger::Info("[BLE] vamos");
    aux->m_client = pClient;
    Logger::Info("[BLE] fois");
  }*/
}

void ClientCallbacks::onMTUChange(NimBLEClient* pClient, uint16_t MTU) {
  Serial.printf("MTU updated: %u\n", MTU);
}

void ClientCallbacks::onDisconnect(NimBLEClient* pClient, int reason){
  Logger::Info("[BLE] my bois is ded");
  Devices::BuzzerToneDuration(400, 300);
  const char *reasonStr = NimBLEUtils::returnCodeToString(reason);
  Logger::Info("[BLE] %s Device disconnected, reason=%s", pClient->getPeerAddress().toString().c_str(), reasonStr);

  xSemaphoreTake(g_remoteControls.m_mutex, portMAX_DELAY);


  auto aux = g_remoteControls.clients[pClient->getPeerAddress().toString()];
  if (aux != nullptr){
    auto svcs = g_remoteControls.GetAcceptedServices();
    for (auto &it : svcs){
      it.second->NotifyDisconnect(aux->getId(), aux->m_controllerId, reasonStr);
    }
    if (aux->m_controllerId < 0xff){
      g_remoteControls.availableIds.push(aux->m_controllerId);
    }
    aux->connected = false;
  }
  
  xSemaphoreGive(g_remoteControls.m_mutex);
}


void ClientCallbacks::onPassKeyEntry(NimBLEConnInfo& connInfo)  {
        Serial.printf("Server Passkey Entry\n");
        /** This should prompt the user to enter the passkey displayed on the peer device. */
        NimBLEDevice::injectPassKey(connInfo, 123456);
  }


void ClientCallbacks::onConfirmPasskey(NimBLEConnInfo& connInfo, uint32_t pass_key) {
        Serial.printf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
}

void ClientCallbacks::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
  if (!connInfo.isEncrypted()) {
    Serial.printf("Encrypt connection failed - disconnecting\n");
    /** Find the client with the connection handle provided in connInfo */
    NimBLEDevice::getClientByHandle(connInfo.getConnHandle())->disconnect();
    return;
  }
}