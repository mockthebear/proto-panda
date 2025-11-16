#include "bluetooth/clientcallbacks.hpp"
#include "bluetooth/ble_client.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

void ClientCallbacks::onConnect(NimBLEClient* pClient) {
  Logger::Info("[BLE] Device Connected");
  auto aux = g_remoteControls.clients[pClient->getPeerAddress().toString()];
  if (aux != nullptr){
    aux->m_client = pClient;
  }
}


void ClientCallbacks::onDisconnect(NimBLEClient* pClient) {
  Devices::BuzzerToneDuration(400, 300);
  Logger::Info("[BLE] Device disconnected");
  xSemaphoreTake(g_remoteControls.m_mutex, portMAX_DELAY);

  auto aux = g_remoteControls.clients[pClient->getPeerAddress().toString()];
  if (aux != nullptr){
    if (aux->m_controllerId < 0xff){
      g_remoteControls.availableIds.push(aux->m_controllerId);
    }
    aux->connected = false;
  }
  
  xSemaphoreGive(g_remoteControls.m_mutex);
}

bool ClientCallbacks::onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params) {
    if(params->itvl_min < 24) { /** 1.25ms units */
        return false;
    } else if(params->itvl_max > 40) { /** 1.25ms units */
        return false;
    } else if(params->latency > 2) { /** Number of intervals allowed to skip */
        return false;
    } else if(params->supervision_timeout > 100) { /** 10ms units */
        return false;
    }
    return true;
}

uint32_t ClientCallbacks::onPassKeyRequest(){
    return 123456;
}

bool ClientCallbacks::onConfirmPIN(uint32_t pass_key){
    return true;
}

void ClientCallbacks::onAuthenticationComplete(ble_gap_conn_desc* desc){
    if(!desc->sec_state.encrypted) {
        NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
        return;
    }
}