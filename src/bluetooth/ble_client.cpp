#include "bluetooth/ble_client.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include "tools/ir.hpp"
#include <Arduino.h>

BleManager* BleManager::m_myself = nullptr;


void AdvertisedDeviceCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
  if (bleObj->canLogDiscoveredClients()){
    Logger::Info("[BLE] Advertised Device found: %s", advertisedDevice->toString().c_str());
  }
  xSemaphoreTake(bleObj->m_mutex, portMAX_DELAY);
  auto acceptedServices = bleObj->GetAcceptedServices();
  for (auto &it : acceptedServices) {
    if(it.second != nullptr && advertisedDevice->isAdvertisingService(it.second->uuid)){
      bool canConnect = true;
      bool matchedTrue = true;
      if (it.second->addrMap.size() > 0){
        canConnect = it.second->addrMap[advertisedDevice->getAddress().toString()];
        matchedTrue = canConnect;
      }
      if (it.second->nameMap.size() > 0){
        canConnect = it.second->nameMap[advertisedDevice->getName()];
        if (!matchedTrue){
          Serial.printf("Expected match name and address. But address failed");
          canConnect = false;
        }
      }

      Logger::Info("Found HID Device: %s\n", advertisedDevice->getName().c_str());
      Logger::Info("Address: %s\n", advertisedDevice->getAddress().toString().c_str());
      if (canConnect){
        bleObj->setScanningMode(false);
        bleObj->toConnect = ConnectionRequest(advertisedDevice, it.second, new BluetoothDeviceHandler());
      }else{
        Logger::Info("Cannot connect because its not present in the addresses");
      }
      xSemaphoreGive(bleObj->m_mutex);
      return;
    }
  }
  xSemaphoreGive(bleObj->m_mutex);
}

void AdvertisedDeviceCallbacks::onScanEnd(const NimBLEScanResults& results, int reason)  {}


bool BleManager::connectToServer(){

  static ClientCallbacks callbacks;
  NimBLEClient* pClient = nullptr;
  const NimBLEAdvertisedDevice* advDevice = toConnect.advertisedDevice;
  BleServiceHandler *handler = toConnect.handler;
  BluetoothDeviceHandler *device = toConnect.deviceHandler;


  device->m_callbacks = &callbacks;
  device->m_device = advDevice;
  
  /** Check if we have a client we should reuse first **/
  
  if (NimBLEDevice::getCreatedClientCount()) {
    pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
    if (pClient) {
      if (!pClient->connect(advDevice, false)) {
        Logger::Info("Failed to reconnect, last error = %d\n", pClient->getLastError());
        return false;
      }
      Logger::Info("Reconnected client\n");
    } else {
      /**
      *  We don't already have a client that knows this device,
      *  check for a client that is disconnected that we can use.
       */
      pClient = NimBLEDevice::getDisconnectedClient();
    }
  }
  /** No client to reuse? Create a new one. */
  
  if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS) {
      Logger::Info("[BLE] Max clients reached - no more connections available\n");
      return false;
  }
  if (!pClient){

    pClient = NimBLEDevice::createClient();

    if (!pClient){
      Logger::Info("[BLE] UNEXPECTED FAILURE, NULL CLIENT\n");
      return false;
    }
    device->m_client = pClient;

    pClient->setClientCallbacks(device->m_callbacks, false);
    pClient->setConnectionParams(24, 24, 0, 150);
    pClient->setConnectTimeout(5 * 1000);
      
    if (!pClient->connect(advDevice, false)) { 
      Logger::Info("Failed to connect, last error = %d\n", pClient->getLastError());
      NimBLEDevice::deleteClient(pClient);
      device->m_client = nullptr; 
      return false;
    }
  }

  Serial.printf("Connected to: %s RSSI: %d, MTU %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi(), pClient->getMTU());
    
  /** Now we can access the HID service characteristics */
  NimBLERemoteService* pSvc = nullptr;

  if (!availableIds.empty()) {
    device->m_controllerId = availableIds.top();
    availableIds.pop();
  } else {
      device->m_controllerId = nextId++;
  }

  
  device->m_client = pClient;
  pSvc = pClient->getService(handler->uuid);
  if (!pSvc) {
      Serial.printf("Service not found!\n");
      pClient->disconnect();

      return false;
  }

  // Look for report characteristics to subscribe to
  // Instead of using getProperties(), we'll try to subscribe and see if it works
  std::vector<BleCharacteristicsHandler*> searchList = handler->getRegisteredCharacteristics();
  std::vector<NimBLERemoteCharacteristic*> pChars = pSvc->getCharacteristics(true);

  for (auto &element : searchList){
    vTaskDelay(1);
    bool matched = false;
    for (auto pChr : pChars) {
        if (pChr->getUUID() == element->uuid){
          matched = true;
          if(pChr->canNotify()) {
            if(!pChr->subscribe(true, element->getLambda(device->getId(), device->m_controllerId))) {
              Logger::Error("[BLE] Characteristics %s in service %s, failed to subscribe.", element->uuid.toString().c_str(), handler->uuid.toString().c_str());
              pClient->disconnect();
              NimBLEDevice::deleteClient(pClient);
              g_remoteControls.availableIds.push(device->m_controllerId);    
    
              return false;
            }else{
              Logger::Error("[BLE] Subscribed on characteristics %s in service %s.", element->uuid.toString().c_str(), pChr->getUUID().toString().c_str());
            }
            continue;
          }
        }
    }
    if (element->required && matched == false){
      Logger::Error("[BLE] Characteristics %s in service %s, is required but not present, dropping. Here follows the list of the avaliable uuids:", element->uuid.toString().c_str(), handler->uuid.toString().c_str());
      std::stringstream ss;
      for (auto pChr : pChars) {
        NimBLEUUID aux = pChr->getUUID();
        ss << aux.to16().toString().c_str() << ", ";
      }
      Logger::Error("[BLE] avaliable characteristics: %s", ss.str().c_str());
      pClient->disconnect();
      NimBLEDevice::deleteClient(pClient);
      g_remoteControls.availableIds.push(device->m_controllerId);  
         
      return false;
    }
  }

  handler->AddDevice(device);

  device->connected = true;

  clients[pClient->getPeerAddress().toString()] = device;
  clientCount++;

  Logger::Info("[BLE] Done with this device! ConnID=%d %s", -1, pClient->getPeerAddress().toString().c_str());
  Devices::BuzzerToneDuration(1500, 300);

  return true;
}


int BleManager::GetClientIdFromControllerId(uint32_t id){
  for (auto &it : clients){
      if (it.second->m_controllerId == id){
        return it.second->getId();
      }   
  }
  return -1;
}

int BleManager::GetRSSI(int clientId){
  for (auto &it : clients){
    if (it.second->getId() == clientId){   
      return it.second->m_client->getRssi();
    }
  }
  return -1;
}

BluetoothDeviceHandler* BleManager::getDeviceById(int clientId){
  for (auto &it : clients){
    if (it.second->getId() == clientId){   
      return it.second;
    }
  }
  return nullptr;
}

void BleManager::setScanningMode(bool mode){
  isScanning = mode;
  m_scanStartAt = millis()+2000;
  if (mode == false){
    Logger::Info("[BLE] Scan stoped");
    NimBLEDevice::getScan()->stop();
    isScanning = false;
    m_canScan = false;
  }else{
    Logger::Info("[BLE] Scan resumed");
    NimBLEDevice::getScan()->clearResults();
    NimBLEDevice::getScan()->start(0, false, false); 
  }
}

BleManager* BleManager::Get(){
  return m_myself;
};

bool BleManager::begin(){
  if (g_InfraRed.IsStarted()){
    return false;
  }
  m_myself = this;
  m_started = true;
  lastScanClearTime = millis();
  return true;
}

bool BleManager::beginRadio(int powerLevel){
  if (!m_started){
    return false;
  }
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
  pScan->setInterval(100);
  pScan->setWindow(100);

  pScan->setActiveScan(true);
  //pScan->start(0);

  return true;
}



void BleManager::sendUpdatesToLua(){
  for (auto &it : handlers){
    it.second->SendMessages();
  }
}

void BleManager::beginScanning(){
  isScanning = false;
  m_canScan = true;
  m_scanStartAt = millis()+1000;
}

void BleManager::AddAcceptedService(std::string name, BleServiceHandler* obj){
  handlers[name] = obj;
  handlersAsync.emplace_back(std::tuple<std::string,BleServiceHandler*>(name, obj));
}

void BleManager::update(){
  if (!m_started){
    return;
  }
  if (millis() - lastScanClearTime >= (30*1000) ) {
    if (isScanning){
      setScanningMode(false);
      NimBLEDevice::getScan()->clearResults(); // Clear the scan results
      Logger::Info("[BLE] Scan results cleared");
      m_canScan = true;
      m_scanStartAt = millis()+1000;
    }
    lastScanClearTime = millis(); // Reset the timer
    Devices::CalculateMemmoryUsage();
  }
  
  
  if (toConnect.ready) {
    connectToServer();
    toConnect.erase();
    m_canScan = true;
    m_scanStartAt = millis()+1000;
    return;
  }

  if (m_canScan){ 
    if (clientCount < maxClients){
      if (!isScanning && !toConnect.ready && m_scanStartAt < millis()){
        setScanningMode(true);
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

        if (clients.find(toErase) != clients.end()){
          NimBLEDevice::deleteClient(clients[toErase]->m_client);
          delete clients[toErase];
          clients.erase(toErase);
          clientCount--;
        }        
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
    if (obj.second && obj.second->m_controllerId == id){
      xSemaphoreGive(m_mutex);
      return true;
    }
  }
  xSemaphoreGive(m_mutex);
  return false;
}