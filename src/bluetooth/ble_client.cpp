#include "bluetooth/ble_client.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

BleManager* BleManager::m_myself = nullptr;

BleSensorHandlerData BleManager::remoteData[MAX_BLE_CLIENTS];

void AdvertisedDeviceCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
  if (bleObj->canLogDiscoveredClients()){
    Logger::Info("[BLE] Advertised Device found: %s", advertisedDevice->toString().c_str());
  }
  auto acceptedServices = bleObj->GetAcceptedServices();
  
  for (auto &it : acceptedServices) {

    if(it.second != nullptr && advertisedDevice->isAdvertisingService(it.second->uuid)){
      Serial.printf("Found HID Device: %s\n", advertisedDevice->getName().c_str());
      Serial.printf("Address: %s\n", advertisedDevice->getAddress().toString().c_str());
      bleObj->setScanningMode(false);
      bleObj->toConnect = ConnectionRequest(advertisedDevice, it.second);
      //toConnect = new ConnectTuple(advertisedDevice, std::get<0>(it), std::get<1>(it), std::get<2>(it));
      return;
    }
  }
}

void AdvertisedDeviceCallbacks::onScanEnd(const NimBLEScanResults& results, int reason)  {
        Serial.printf("Scan Ended, reason: %d, device count: %d; Restarting scan\n", reason, results.getCount());
        NimBLEDevice::getScan()->start(0, false, true);
    }


void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  if (pRemoteCharacteristic == nullptr){
    return;
  }
  g_remoteControls.AddMessageToQueue(pRemoteCharacteristic, pData, length, isNotify);
    /*

    int16_t *data16 = &((int16_t*)pData)[7];
    int id = ((uint8_t*)data16)[0];    
    if (id >= 0 && id < MAX_BLE_CLIENTS){
      BleManager::remoteData[id].copy(data);
      BleManager::remoteData[id].setLastUpdate();
    }*/


  //}
}



bool BleManager::connectToServer(){
  static ClientCallbacks callbacks;
  NimBLEClient* pClient = nullptr;
  const NimBLEAdvertisedDevice* advDevice = toConnect.advertisedDevice;
  BleServiceHandler *handler = toConnect.handler;

  BluetoothDeviceHandler *device = new BluetoothDeviceHandler();
  device->m_callbacks = &callbacks;
  device->m_device = advDevice;
  
  /** Check if we have a client we should reuse first **/

  if (NimBLEDevice::getCreatedClientCount()) {
      pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
      if (pClient) {
        device->m_client = pClient;
        if (!pClient->connect(advDevice, false)) {
            Logger::Info("[BLE] Fail on 1");
            return false;
        }
        Logger::Info("[BLE] Reconnected client");
      } else {
          pClient = NimBLEDevice::getDisconnectedClient();
      }
  }

  /** No client to reuse? Create a new one. */
  if (!pClient) {
      if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS) {
          Logger::Info("[BLE] Max clients reached - no more connections available\n");
          return false;
      }

      pClient = NimBLEDevice::createClient();
      device->m_client = pClient;
    
      Logger::Info("[BLE] New client created\n");

      
      
      pClient->setClientCallbacks(device->m_callbacks, false);
      pClient->setConnectionParams(12, 12, 0, 150);
      pClient->setConnectTimeout(5 * 1000);

        if (!pClient->connect(advDevice)) {
            NimBLEDevice::deleteClient(pClient);
            Serial.printf("Failed to connect, deleted client 1\n");
            delete device;
            return false;
        }
    }

    if (!pClient->isConnected()) {
      if (!pClient->connect(advDevice)) {
          Serial.printf("Failed to connect 2\n");
          delete device;
          return false;
      }
    }

    Serial.printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    /** Now we can access the HID service characteristics */
    NimBLERemoteService* pSvc = nullptr;

    if (!availableIds.empty()) {
    device->m_controllerId = availableIds.top();
    availableIds.pop();
  } else {
      device->m_controllerId = nextId++;
  }

  

  pSvc = pClient->getService(handler->uuid);
  if (!pSvc) {
      Serial.printf("HID service not found!\n");
      pClient->disconnect();
      return false;
  }

  Serial.printf("Found HID service\n");
  // Look for report characteristics to subscribe to
  // Instead of using getProperties(), we'll try to subscribe and see if it works
  std::vector<BleCharacteristicsHandler*> searchList = handler->getCharacteristics();
  std::vector<NimBLERemoteCharacteristic*> pChars = pSvc->getCharacteristics(true);
  for (auto &element : searchList){
    vTaskDelay(1);
    bool matched = false;
    for (auto pChr : pChars) {
        if (pChr->getUUID() == element->uuid){
          matched = true;
          if(pChr->canNotify()) {
            if(!pChr->subscribe(true, notifyCB)) {
              Logger::Error("[BLE] Characteristics %s in service %s, failed to subscribe, dropping.", element->uuid.toString().c_str(), handler->uuid.toString().c_str());
              pClient->disconnect();
              NimBLEDevice::deleteClient(pClient);
              g_remoteControls.availableIds.push(device->m_controllerId);         
              delete device;
              return false;
            }else{
              Logger::Error("[BLE] Subscribed on characteristics %s in service %s, failed to subscribe.", element->uuid.toString().c_str(), pChr->getUUID().toString().c_str());
            }
            continue;
          }
        }
    }
    if (element->required && matched == false){
      Logger::Error("[BLE] Characteristics %s in service %s, is required but not present, dropping.", element->uuid.toString().c_str(), handler->uuid.toString().c_str());
      pClient->disconnect();
      NimBLEDevice::deleteClient(pClient);
      g_remoteControls.availableIds.push(device->m_controllerId);         
      delete device;
      return false;
    }
  }

  device->connected = true;
  Serial.printf("iz complete\n");
  
  clients[pClient->getPeerAddress().toString()] = device;
  clientCount++;
  Logger::Info("[BLE] Done with this device! ConnID=%d %s", -1, pClient->getPeerAddress().toString().c_str());
  Devices::BuzzerToneDuration(1500, 300);

  return true;
}


void BleManager::setScanningMode(bool mode){
  if (mode == false){
    NimBLEDevice::getScan()->stop();
  }
  isScanning = mode;
}

BleManager* BleManager::Get(){
  return m_myself;
};

bool BleManager::begin(){

  m_myself = this;
  
  for (int i = 0; i<MAX_BLE_CLIENTS;i++){
    remoteData[i] = BleSensorHandlerData();
  }

  

  m_started = true;
  
  return true;
}

bool BleManager::beginRadio(){
  NimBLEDevice::init("Protopanda");
  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

  NimBLEScan* pScan = NimBLEDevice::getScan();
  if (!pScan){
    return false;
  }

  AdvertisedDeviceCallbacks *cb = new AdvertisedDeviceCallbacks();
  if (!cb){
    return false;
  }
  cb->bleObj = this;

  pScan->setScanCallbacks(cb, false);

    /** Set scan interval (how often) and window (how long) in milliseconds */
  pScan->setInterval(45);
  pScan->setWindow(15);

  pScan->setActiveScan(true);
  pScan->start(0);

  return true;
}




void BleManager::AddMessageToQueue(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  const NimBLERemoteService* svc = pRemoteCharacteristic->getRemoteService();
  if (svc == nullptr){
    Logger::Info("[BLE] noooo1");
    return; 
  }

  static std::map<std::string,bool> warnedMap;

  std::string name = svc->getUUID().toString();
  BleServiceHandler* svcHandler = handlers[name];
  if (svcHandler != nullptr){
    svcHandler->AddMessage(pRemoteCharacteristic->getUUID(), pData, length, isNotify);
  }else{
    if (warnedMap[name] == false){
      Logger::Error("Message arrived on service unmapped with service %s ", name.c_str());
      warnedMap[name] = true;
    }
  }
  /*NimBLEClient* client =  svc->getClient();
  if (client == nullptr){
    Logger::Info("[BLE] noooo 2");
    return;
  }*/

//void BleManager::AddMessageToQueue(NimBLEUUID svcUUID, NimBLEUUID charUUID,NimBLEAddress addr, uint8_t* pData, size_t length, bool isNotify){
  //TODO: what top do with the addr? send to lua? does it need?
  //xSemaphoreTake(m_mutex, portMAX_DELAY);
  //
  //xSemaphoreGive(m_mutex);
 // if (svcHandler != nullptr){
    //Logger::Error("Adding message to %s", svcUUID.toString().c_str());
    //svcHandler->AddMessage(charUUID, pData, length, isNotify);
  //}else{
    //Logger::Error("Message arrived on service unmapped with lenght %d and i am %d ",length, (int)this);
  //}
}


void BleSensorHandlerData::updateButtons(){
  for (int i=0;i<MAX_BLE_BUTTONS;i++){
    if (real_inputButtonsStatus[i] == BUTTON_JUST_PRESSED){
      real_inputButtonsStatus[i] = BUTTON_PRESSED;
    }
    if (real_inputButtonsStatus[i] == BUTTON_JUST_RELEASED){
      real_inputButtonsStatus[i] = BUTTON_RELEASED;
    }
    if (buttons[i] != last_inputButtonsStatus[i]){
      if (buttons[i] == 1){
        real_inputButtonsStatus[i] = BUTTON_JUST_PRESSED;
      }else{
        real_inputButtonsStatus[i] = BUTTON_JUST_RELEASED;
      }
      last_inputButtonsStatus[i] = buttons[i];
    }
  }
}

void BleManager::sendUpdatesToLua(){
  for (auto &it : handlers){
    it.second->SendMessages();
  }
}

void BleManager::updateButtons(){
  for (int i = 0; i<MAX_BLE_CLIENTS;i++){
    remoteData[i].updateButtons();
  }
}

void BleManager::beginScanning(){
  isScanning = false;
  m_canScan = true;
}

void BleManager::AddAcceptedService(std::string name, BleServiceHandler* obj){
  handlers[name] = obj;
}

void BleManager::update(){
  if (!m_started){
    return;
  }

  
  if (millis() - lastScanClearTime >= (30*1000) ) {
    if (!isScanning){
      //NimBLEDevice::getScan()->clearDuplicateCache();
      NimBLEDevice::getScan()->clearResults(); // Clear the scan results
      Logger::Info("[BLE] Scan results cleared");
    }
    lastScanClearTime = millis(); // Reset the timer
    Devices::CalculateMemmoryUsage();
  }
  
  
  if (toConnect.ready) {
    connectToServer();
    toConnect.erase();
    return;
  }

  if (m_canScan){
    if (clientCount < maxClients){
      if (!isScanning){
        isScanning = true;
        Logger::Info("[BLE] Scan resumed");
        //NimBLEDevice::getScan()->clearDuplicateCache();
        NimBLEDevice::getScan()->clearResults();
        NimBLEDevice::getScan()->start(0);
      }
    }
      
    if (clientCount > 0){
      std::string toErase;
      xSemaphoreTake(m_mutex, portMAX_DELAY);
      for (auto &aux : clients){
        if (!aux.second->connected){
            toErase = aux.first;
        }
      }
        
      if (toErase.size() > 0){
        Logger::Info("[BLE] Aw man, we're killing it");
        NimBLEDevice::deleteClient(clients[toErase]->m_client);
        delete clients[toErase];
        clients.erase(toErase);
        clientCount--;
      }
      xSemaphoreGive(m_mutex);
    }
  }

}


bool BleManager::hasChangedClients(){
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  if (clientCount != clients.size()){
    clientCount = clients.size();
    xSemaphoreGive(m_mutex);
    return true;
  }
  xSemaphoreGive(m_mutex);
  return false;
}

bool BleManager::isElementIdConnected(int id){
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  for (auto &obj : clients){
    if (obj.second->m_controllerId == id){
      xSemaphoreGive(m_mutex);
      return true;
    }
  }
  xSemaphoreGive(m_mutex);
  return false;
}