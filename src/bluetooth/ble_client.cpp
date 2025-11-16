#include "bluetooth/ble_client.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>



void scanEndedCB(NimBLEScanResults results){
  Logger::Info("[BLE] Scan Ended");
}


//static ConnectTuple *toConnect = nullptr;


BleManager* BleManager::m_myself = nullptr;

BleSensorHandlerData BleManager::remoteData[MAX_BLE_CLIENTS];



void AdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
  if (bleObj->canLogDiscoveredClients()){
    Logger::Info("[BLE] Advertised Device found: %s", advertisedDevice->toString().c_str());
  }


  auto acceptedServices = bleObj->GetAcceptedServices();
  
  for (auto &it : acceptedServices) {

    if(it.second != nullptr && advertisedDevice->isAdvertisingService(it.second->uuid)){
      bleObj->setScanningMode(false);
      bleObj->toConnect = ConnectionRequest(advertisedDevice, it.second);
      //toConnect = new ConnectTuple(advertisedDevice, std::get<0>(it), std::get<1>(it), std::get<2>(it));
      return;
    }
  }
}



void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  //BleSensorData* data = (BleSensorData*)pData;

  if (pRemoteCharacteristic == nullptr){
    return;
  }

  NimBLERemoteService* svc = pRemoteCharacteristic->getRemoteService();
  if (svc == nullptr){
    Logger::Info("[BLE] noooo1");
    return;
  }
  NimBLEClient* client =  svc->getClient();
  if (client == nullptr){
    Logger::Info("[BLE] noooo 2");
    return;
  }
  BleManager::Get()->AddMessageToQueue(pRemoteCharacteristic->getRemoteService()->getUUID(), pRemoteCharacteristic->getUUID(), client->getPeerAddress(), pData, length, isNotify);

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

  NimBLEClient* pClient = nullptr;
  NimBLEAdvertisedDevice* advDevice = toConnect.advertisedDevice;
  BleServiceHandler *handler = toConnect.handler;


  BluetoothDeviceHandler *device = new BluetoothDeviceHandler();
  device->m_callbacks = new ClientCallbacks();
  device->m_callbacks->bleObj = this;

  /** Check if we have a client we should reuse first **/
  if(NimBLEDevice::getClientListSize()) {
    pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
    if(pClient){
        if(!pClient->connect(advDevice, false)) {
          Logger::Info("[BLE] Fail on 1");
          delete device;
          return false;
        }
    }else {
        pClient = NimBLEDevice::getDisconnectedClient();
    }
  }
  /** No client to reuse? Create a new one. */
  if(!pClient) {
    if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
        Logger::Info("[BLE] Fail on 2");
        delete device;
        return false;
    }

    pClient = NimBLEDevice::createClient();


    pClient->setClientCallbacks(device->m_callbacks, false);    
    pClient->setConnectionParams(12,12,0,51);
    pClient->setConnectTimeout(5);

    if (!pClient->connect(advDevice)) {
        NimBLEDevice::deleteClient(pClient);
        Logger::Info("[BLE] Fail on 3");
        delete device;
        return false;
    }
  }


  if(!pClient->isConnected()) {
    if (!pClient->connect(advDevice)) {
      Logger::Info("[BLE] Failed to connect");
      delete device;
      return false;
    }
  }

  device->m_client = pClient;
  device->m_device = advDevice;

  

  if (!pClient->discoverAttributes()) {
    Logger::Info("[BLE] Attribute discovery failed!");
    delete device;
    return false;
  }

  if (!availableIds.empty()) {
    device->m_controllerId = availableIds.top();
    availableIds.pop();
  } else {
      device->m_controllerId = nextId++;
  }

  NimBLERemoteService* pSvc = nullptr;
  NimBLERemoteCharacteristic* pChr = nullptr;


  

  pSvc = pClient->getService(handler->uuid);
  if(!pSvc) { 
    Logger::Error("[BLE] For some reason, client dont seems to have the required service.");
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    g_remoteControls.availableIds.push(device->m_controllerId);
    delete device;
    return false; 
  }

  std::vector<BleCharacteristicsHandler*> searchList = handler->getCharacteristics();

  for (auto &element : searchList){
    pChr = pSvc->getCharacteristic(element->uuid);
    if (!pChr){
      if (!element->required){
        continue;
      }
      Logger::Error("[BLE] Not found required characteristics %s in service %s, dropping.", element->uuid.toString().c_str(), handler->uuid.toString().c_str());
      pClient->disconnect();
      NimBLEDevice::deleteClient(pClient);
      g_remoteControls.availableIds.push(device->m_controllerId);
      delete device;
      return false;
    }

    if (element->temporary_send_legacy_packet){
      delay(300);
      if(pChr && pChr->canWrite()) {
        if (!pChr->writeValue((uint8_t*)&device->m_controllerId, sizeof(uint32_t), true)){
          Logger::Info("[BLE] Fail to sned woah");
        }else{
          Logger::Info("[BLE] Sent controller id: %d", device->m_controllerId);
        }
      }
    }

    if (element->notify){
      if(pChr->canNotify()) {
        if(!pChr->subscribe(true, notifyCB)) {
          Logger::Error("[BLE] Characteristics %s in service %s, failed to subscribe, dropping.", element->uuid.toString().c_str(), handler->uuid.toString().c_str());
          pClient->disconnect();
          NimBLEDevice::deleteClient(pClient);
          g_remoteControls.availableIds.push(device->m_controllerId);         
          delete device;
          return false;
        }else{
          Logger::Info("[BLE] subscribed on meme");
        }
      }else{
        Logger::Error("[BLE] Characteristics %s in service %s, does not have notify, dropping.", element->uuid.toString().c_str(), handler->uuid.toString().c_str());
        pClient->disconnect();
        NimBLEDevice::deleteClient(pClient);
        g_remoteControls.availableIds.push(device->m_controllerId);
        delete device;
        return false;
      }
    }
  }
  device->connected = true;
  clients[pClient->getPeerAddress().toString()] = device;
  clientCount++;

  Logger::Info("[BLE] Done with this device! ConnID=%d %s", pClient->getConnId(), pClient->getPeerAddress().toString().c_str());
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
  NimBLEDevice::init("");
  NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
  m_myself = this;
#ifdef ESP_PLATFORM
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
#else
    NimBLEDevice::setPower(9); /** +9db */
#endif

  NimBLEScan* pScan = NimBLEDevice::getScan();
  if (!pScan){
    return false;
  }
  for (int i = 0; i<MAX_BLE_CLIENTS;i++){
    remoteData[i] = BleSensorHandlerData();
  }

  AdvertisedDeviceCallbacks *cb = new AdvertisedDeviceCallbacks();
  if (!cb){
    return false;
  }
  cb->bleObj = this;

  pScan->setAdvertisedDeviceCallbacks(cb);

    /** Set scan interval (how often) and window (how long) in milliseconds */
  pScan->setInterval(45);
  pScan->setWindow(15);

  pScan->setActiveScan(true);
  /** Start scanning for advertisers for the scan time specified (in seconds) 0 = forever
  *  Optional callback for when scanning stops.
  */  
  m_started = true;
  NimBLEDevice::getScan()->start(0, scanEndedCB);
  return true;
}





void BleManager::AddMessageToQueue(NimBLEUUID &&svcUUID, NimBLEUUID &&charUUID, NimBLEAddress &&addr, uint8_t* pData, size_t length, bool isNotify){
  //TODO: what top do with the addr? send to lua? does it need?
  BleServiceHandler* svcHandler = handlers[svcUUID.toString()];
  if (svcHandler != nullptr){
    svcHandler->AddMessage(charUUID, pData, length, isNotify);
  }else{
    Logger::Error("Message arrived on service unmapped uuid %s with characteristics %s and lenght %d", svcUUID.toString().c_str(), charUUID.toString().c_str(), length);
  }
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

void BleManager::update(){
  if (!m_started){
    return;
  }
  
  if (millis() - lastScanClearTime >= (30*1000) ) {
    NimBLEDevice::getScan()->clearDuplicateCache();
    NimBLEDevice::getScan()->clearResults(); // Clear the scan results
    lastScanClearTime = millis(); // Reset the timer
    Logger::Info("[BLE] Scan results cleared");
    Devices::CalculateMemmoryUsage();
  }
  
  if (toConnect.advertisedDevice != nullptr) {
    connectToServer();
    toConnect.erase();
    return;
  }

  if (m_canScan){
    if (clientCount < maxClients){
      if (!isScanning){
        isScanning = true;
        NimBLEDevice::getScan()->clearDuplicateCache();
        NimBLEDevice::getScan()->clearResults();
        NimBLEDevice::getScan()->start(0, scanEndedCB);
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

int BleManager::acceptTypes(std::string service, std::string characteristicStream, std::string characteristicId){

  // Validate input lengths
  if (service.length() != 36 ) {
    Logger::Info("[BLE] Malformed servie UUID");
    return -1;
  }
  if (characteristicStream.length() != 4) {
    Logger::Info("[BLE] Malformed characteristicStream UUID");
    return -1;
  }

  if (characteristicId.length() != 4) {
    Logger::Info("[BLE] Malformed characteristicId UUID");
    return -1;
  }

  std::string modified1 = service;
  modified1.replace(4, 4, characteristicStream); 

  std::string modified2 = service;
  modified2.replace(4, 4, characteristicId); 

  NimBLEUUID servUUID(service);
  NimBLEUUID streamUUID(modified1);
  NimBLEUUID idUUID(modified2);

  Logger::Info("[BLE] Following characteristics are beeing listen:\nService: %s\nStream: %s\nID: %s\n", servUUID.toString().c_str(), streamUUID.toString().c_str(), idUUID.toString().c_str());

  BleServiceHandler *hand = new BleServiceHandler(servUUID);
  hand->AddCharacteristics(streamUUID);
  hand->AddCharacteristics_TMP(idUUID);
  handlers[servUUID.toString()] = hand;
  return 0;
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