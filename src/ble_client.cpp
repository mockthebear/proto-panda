#include "ble_client.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

void scanEndedCB(NimBLEScanResults results){
  Logger::Info("[BLE] Scan Ended");
}

static bool isScanning = false;

static ConnectTuple *toConnect = nullptr;


BleManager* BleManager::m_myself = nullptr;

BleSensorHandlerData BleManager::remoteData[MAX_BLE_CLIENTS];


void ClientCallbacks::onConnect(NimBLEClient* pClient) {
  Logger::Info("[BLE] Device Connected");
}


void ClientCallbacks::onDisconnect(NimBLEClient* pClient) {
  //Logger::Info("[BLE] Device disconnected %s", pClient->getPeerAddress().toString().c_str());
  Devices::BuzzerToneDuration(400, 300);
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

void AdvertisedDeviceCallbacks::onResult(NimBLEAdvertisedDevice* advertisedDevice) {
  if (bleObj->canLogDiscoveredClients()){
    Logger::Info("[BLE] Advertised Device found: %s", advertisedDevice->toString().c_str());
  }
  auto vecOfTuples = bleObj->GetAcceptedUUIDS();
  for (size_t i = 0; i < vecOfTuples.size(); ++i) {
    const auto it = vecOfTuples[i];

    if(advertisedDevice->isAdvertisingService(std::get<0>(it))){
      NimBLEDevice::getScan()->stop();
      isScanning = false;
      toConnect = new ConnectTuple(advertisedDevice, std::get<0>(it), std::get<1>(it), std::get<2>(it));
      return;
    }
  }
}


void BleServiceHandler::AddMessage(uint8_t* pData, size_t length, bool isNotify){
 xSemaphoreTake(queueMutex, portMAX_DELAY);
  if (dataQueue.size() > 100) {
    dataQueue.pop();
  }
  dataQueue.emplace(pData, length);
  xSemaphoreGive(queueMutex);
}


void BleManager::AddMessageToQueue(NimBLEUUID &&svcUUID, NimBLEUUID &&charUUID, uint8_t* pData, size_t length, bool isNotify){
  
  BleServiceHandler* svcHandler = handlers[svcUUID];
  if (svcHandler != nullptr){
    svcHandler->AddMessage(pData, length, isNotify);
  }else{
    Logger::Error("Message arrived on service unmapped uuid %s with characteristics %s and lenght %d", svcUUID.toString().c_str(), charUUID.toString().c_str(), length);
  }
}


void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  //BleSensorData* data = (BleSensorData*)pData;

  if (pRemoteCharacteristic != nullptr){

    
    BleManager::Get()->AddMessageToQueue(pRemoteCharacteristic->getRemoteService()->getUUID(), pRemoteCharacteristic->getUUID(), pData, length, isNotify);

    /*

    int16_t *data16 = &((int16_t*)pData)[7];
    int id = ((uint8_t*)data16)[0];    
    if (id >= 0 && id < MAX_BLE_CLIENTS){
      BleManager::remoteData[id].copy(data);
      BleManager::remoteData[id].setLastUpdate();
    }*/


  }
}

bool BleManager::connectToServer(ConnectTuple *tlp){
  NimBLEClient* pClient = nullptr;
  NimBLEAdvertisedDevice* advDevice = tlp->m_device;
  if(NimBLEDevice::getClientListSize()) {
    pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
    if(pClient){
        if(!pClient->connect(advDevice, false)) {
            tlp->connected = false;
            return false;
        }
    }else {
        pClient = NimBLEDevice::getDisconnectedClient();
    }
  }
  if(!pClient) {
    if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
        tlp->connected = false;
        return false;
    }

    pClient = NimBLEDevice::createClient();
    tlp->callbacks = new ClientCallbacks();
    tlp->callbacks->bleObj = this;
    pClient->setClientCallbacks(tlp->callbacks, false);
    /** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
      *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
     *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
     *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
     */
    pClient->setConnectionParams(12,12,0,51);
    /** Set how long we are willing to wait for the connection to complete (seconds), default is 30. */
    pClient->setConnectTimeout(5);

    if (!pClient->connect(advDevice)) {
        NimBLEDevice::deleteClient(pClient);
        tlp->connected = false;
        return false;
    }
  }
  if(!pClient->isConnected()) {
    if (!pClient->connect(advDevice)) {
      Logger::Info("[BLE] Failed to connect");
      tlp->connected = false;
      return false;
    }
  }

  delay(300);

  if (!availableIds.empty()) {
    tlp->m_controllerId = availableIds.top();
    availableIds.pop();
  } else {
      tlp->m_controllerId = nextId++;
  }

  NimBLERemoteService* pSvc = nullptr;
  NimBLERemoteCharacteristic* pChr = nullptr;
  NimBLERemoteCharacteristic* pConfigChr = nullptr;

  pSvc = pClient->getService(tlp->m_service);
  if(pSvc) {    
    pConfigChr = pSvc->getCharacteristic(tlp->m_idCharacteristic);
    if(pConfigChr && pConfigChr->canWrite()) {
        pConfigChr->writeValue((uint8_t*)&tlp->m_controllerId, sizeof(uint32_t), true);
        Logger::Info("[BLE] Sent controller id: %d", tlp->m_controllerId);
    }else{
      Logger::Error("[BLE] Missign write characteristics");
      pClient->disconnect();
      NimBLEDevice::deleteClient(pClient);
      tlp->connected = false;
      return false;
    }
    delay(100);

    pChr = pSvc->getCharacteristic( (tlp->m_streamCharacteristic) );
    if(pChr) {
      if(pChr->canNotify()) {
          if(!pChr->subscribe(true, notifyCB)) {
            Logger::Error("[BLE] The notification is a sussy baka");
            tlp->connected = false;
            NimBLEDevice::deleteClient(pClient);
            g_remoteControls.availableIds.push(tlp->m_controllerId);
            pClient->disconnect();
            return false;
          }
      }else{
        Logger::Error("[BLE] ye, there is not there wtf man");
        pClient->disconnect();
        g_remoteControls.availableIds.push(tlp->m_controllerId);
        NimBLEDevice::deleteClient(pClient);
        tlp->connected = false;
        return false;
      }
    }
  }else{
    Logger::Error("[BLE] The service is a sussy baka");
    pClient->disconnect();
    NimBLEDevice::deleteClient(pClient);
    g_remoteControls.availableIds.push(tlp->m_controllerId);
    tlp->connected = false;
    return false;
  }

  tlp->m_client = pClient;
  clients[pClient->getPeerAddress().toString()] = tlp;
  clientCount++;

  Logger::Info("[BLE] Done with this device! ConnID=%d %s", pClient->getConnId(), pClient->getPeerAddress().toString().c_str());
  Devices::BuzzerToneDuration(1500, 300);
  return true;
}

BleManager* BleManager::Get(){
  return m_myself
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
  
  if (toConnect != nullptr) {
    connectToServer(toConnect);
    toConnect = nullptr;
  }else{
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
          NimBLEDevice::deleteClient(clients[toErase]->m_client);
          delete clients[toErase]->callbacks;
          delete clients[toErase];
          clients.erase(toErase);
          clientCount--;
        }
        xSemaphoreGive(m_mutex);
      }
      //
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

  m_acceptedUUIDs.emplace_back(std::make_tuple(servUUID, streamUUID, idUUID));
  return m_acceptedUUIDs.size() -1;
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