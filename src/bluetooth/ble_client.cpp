#include "bluetooth/ble_client.hpp"
#ifdef ENABLE_BLE
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include "tools/ir.hpp"
#include <Arduino.h>

BleManager* BleManager::m_myself = nullptr;


void BleManager::RequestConnection(const NimBLEAdvertisedDevice* advertisedDevice, BleServiceHandler* handler){
  setScanningMode(false);
  toConnect = ConnectionRequest(
    advertisedDevice->getAddress().toString(),
    advertisedDevice->getAddress().getType(),
    advertisedDevice->getName(),
    handler,
    new BluetoothDeviceHandler()
  );
}

bool BleManager::TryConnectByAddress(const NimBLEAdvertisedDevice* advertisedDevice){
  auto pairedDevices = GetPairedDevices();
  auto it = pairedDevices.find(advertisedDevice->getAddress().toString());
  if (it == pairedDevices.end()){
    return false;
  }

  BleServiceHandler* handler = it->second;

  if (!toConnect.ready){
    RequestConnection(advertisedDevice, handler);
  }else{
    Logger::Info("Cannot connect because its not ready");
  }
  return true; 
}

bool BleManager::TryConnectByService(const NimBLEAdvertisedDevice* advertisedDevice){
  auto acceptedServices = GetAcceptedServices();
  for (auto &it : acceptedServices) {
    if (it.second == nullptr || !advertisedDevice->isAdvertisingService(it.second->uuid)){
      continue;
    }

    BleServiceHandler* handler = it.second;
    bool canConnect = true;
    bool matchedTrue = true;

    if (handler->nameMap.size() > 0){
      auto nameIt = handler->nameMap.find(advertisedDevice->getName());
      canConnect = (nameIt != handler->nameMap.end()) && nameIt->second;
      if (!matchedTrue){
        Serial.printf("Expected match name and address. But address failed");
        canConnect = false;
      }
    }

    Logger::Info("Found Device: %s", advertisedDevice->getName().c_str());
    Logger::Info("Address: %s\n", advertisedDevice->getAddress().toString().c_str());

    if (canConnect && !toConnect.ready){
      RequestConnection(advertisedDevice, handler);
    }else{
      Logger::Info("Cannot connect because its not present in the addresses");
    }
    return true; // matched a service entry, caller should stop searching
  }
  return false; // no accepted service matched
}


void AdvertisedDeviceCallbacks::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
  if (bleObj->canLogDiscoveredClients()){
    Logger::Info("[BLE][ByAddress=%d] Advertised Device found: %s", bleObj->IsScanningByAddress(), advertisedDevice->toString().c_str());
  }

  xSemaphoreTake(bleObj->m_mutex, portMAX_DELAY);

  if (bleObj->IsScanningByAddress()){
    bleObj->TryConnectByAddress(advertisedDevice);
  }else{
    bleObj->TryConnectByService(advertisedDevice);
  }

  xSemaphoreGive(bleObj->m_mutex);
}

void AdvertisedDeviceCallbacks::onScanEnd(const NimBLEScanResults& results, int reason)  {
  
}


bool BleManager::connectToServer(){

  static ClientCallbacks callbacks;
  NimBLEClient* pClient = nullptr;
  NimBLEAddress peerAddr(toConnect.address, toConnect.addressType);
  BleServiceHandler *handler = toConnect.handler;
  BluetoothDeviceHandler *device = toConnect.deviceHandler;
  bool idAllocated = false;

  device->m_callbacks = &callbacks;
  device->m_deviceName = toConnect.name;

  /** Releases everything this attempt acquired. The device handler is only handed
   *  over to the service handler on success, so on failure it is ours to free. */
  auto fail = [&](const char* reason) -> bool {
    Logger::Error("[BLE] Connection to %s aborted: %s", peerAddr.toString().c_str(), reason);
    if (pClient){
      if (pClient->isConnected()){
        pClient->disconnect();
      }
      NimBLEDevice::deleteClient(pClient);
      pClient = nullptr;
    }
    if (idAllocated){
      availableIds.push(device->m_controllerId);
    }
    delete device;
    return false;
  };

  /** Reuse a client that already knows this peer, or any idle one. */
  if (NimBLEDevice::getCreatedClientCount()) {
    pClient = NimBLEDevice::getClientByPeerAddress(peerAddr);
    if (!pClient) {
      pClient = NimBLEDevice::getDisconnectedClient();
    }
  }

  /** No client to reuse? Create a new one. */
  if (!pClient){
    if (NimBLEDevice::getCreatedClientCount() >= NIMBLE_MAX_CONNECTIONS) {
      return fail("max clients reached - no more connections available");
    }
    pClient = NimBLEDevice::createClient();
    if (!pClient){
      return fail("unexpected failure, null client");
    }
    pClient->setConnectionParams(24, 24, 0, 150);
    pClient->setConnectTimeout(5 * 1000);
  }

  pClient->setClientCallbacks(&callbacks, false);
  device->m_client = pClient;

  /** A reused client may still be disconnected, so always check before connecting. */
  if (!pClient->isConnected()){
    if (!pClient->connect(peerAddr)) {
      Logger::Info("Failed to connect, last error = %d\n", pClient->getLastError());
      return fail("could not connect");
    }
  }

  Serial.printf("Connected to: %s RSSI: %d, MTU %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi(), pClient->getMTU());

  /** Pair / bond / encrypt the link. HID over GATT requires this before the
   *  report characteristics can be subscribed. If a bond already exists the
   *  stored keys are used and no user interaction is needed. */
  if (handler->encryptionRequired){
    Logger::Info("[BLE] Securing connection with %s", pClient->getPeerAddress().toString().c_str());
    if (!pClient->secureConnection()){
      return fail("pairing/encryption failed");
    }
  }

  if (!availableIds.empty()) {
    device->m_controllerId = availableIds.top();
    availableIds.pop();
  } else {
    device->m_controllerId = nextId++;
  }
  idAllocated = true;

  NimBLERemoteService* pSvc = pClient->getService(handler->uuid);
  if (!pSvc) {
    return fail("service not found");
  }

  // Look for report characteristics to subscribe to
  std::vector<BleCharacteristicsHandler*> searchList = handler->getRegisteredCharacteristics();
  std::vector<NimBLERemoteCharacteristic*> pChars = pSvc->getCharacteristics(true);

  for (auto &element : searchList){
    vTaskDelay(1);
    bool matched = false;
    for (auto pChr : pChars) {
      NimBLEUUID chrUuid = pChr->getUUID();
      chrUuid.to128(); /** element->uuid is always stored as 128 bit, compare like with like */
      if (!(chrUuid == element->uuid)){
        continue;
      }
      matched = true;
      if (!pChr->canNotify()){
        continue; /** e.g. HID output/feature reports */
      }
      /** A HID device can expose several characteristics with the same UUID (0x2A4D), all of them are subscribed. */
      if (!pChr->subscribe(true, element->getLambda(device->getId(), device->m_controllerId))) {
        Logger::Error("[BLE] Characteristics %s (handle 0x%04X) in service %s, failed to subscribe.", element->uuid.toString().c_str(), pChr->getHandle(), handler->uuid.toString().c_str());
        return fail("failed to subscribe");
      }
      Logger::Info("[BLE] Subscribed on characteristics %s (handle 0x%04X) in service %s.", element->uuid.toString().c_str(), pChr->getHandle(), handler->uuid.toString().c_str());
    }
    if (element->required && matched == false){
      Logger::Error("[BLE] Characteristics %s in service %s, is required but not present, dropping. Here follows the list of the avaliable uuids:", element->uuid.toString().c_str(), handler->uuid.toString().c_str());
      std::stringstream ss;
      for (auto pChr : pChars) {
        NimBLEUUID aux = pChr->getUUID();
        ss << aux.to16().toString().c_str() << ", ";
      }
      Logger::Error("[BLE] avaliable characteristics: %s", ss.str().c_str());
      return fail("required characteristic not present");
    }
  }

  handler->AddDevice(device);

  device->connected = true;

  device->m_deviceAddress = pClient->getPeerAddress().toString();

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
    isScanning = false;
    m_canScan = false;
  }else{
    Logger::Info("[BLE] Scan resuming");
    NimBLEDevice::getScan()->clearResults();
    Logger::Info("[BLE] Scan resumed");
    NimBLEDevice::getScan()->start(0, false, true); 
  }
}

void BleManager::applySecuritySettings(){
  NimBLEDevice::setSecurityIOCap(m_ioCap);
  NimBLEDevice::setSecurityAuth(m_bonding, m_mitm, m_secureConn);
}

void BleManager::setSecurityIOCap(int cap){
  if (cap < BLE_HS_IO_DISPLAY_ONLY || cap > BLE_HS_IO_KEYBOARD_DISPLAY){
    Logger::Error("[BLE] Invalid IO capability %d", cap);
    return;
  }
  m_ioCap = (uint8_t)cap;
  if (m_radioStarted){
    NimBLEDevice::setSecurityIOCap(m_ioCap);
  }
}

void BleManager::setSecurityAuth(bool bonding, bool mitm, bool sc){
  m_bonding = bonding;
  m_mitm = mitm;
  m_secureConn = sc;
  if (m_radioStarted){
    NimBLEDevice::setSecurityAuth(m_bonding, m_mitm, m_secureConn);
  }
}

void BleManager::setSecurityPasskey(uint32_t pin){
  if (pin > 999999){
    Logger::Error("[BLE] Passkey must be between 0 and 999999");
    return;
  }
  m_passkey = pin;
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
  m_radioStarted = true;
  applySecuritySettings(); /** IO capability + auth flags, configurable from Lua before/after this call */
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
  pScan->setInterval(200);
  pScan->setWindow(80);

  pScan->setActiveScan(true);
  //pScan->start(0);

  return true;
}



void BleManager::sendUpdatesToLua(){
  for (auto &it : pairedHandlers){
    it.second->SendMessages();
  }
  for (auto &it : handlers){
    it.second->SendMessages();
  }
}

bool BleManager::beginScanning(){
  if (isScanning){
    return false;
  }
  isScanning = false;
  m_canScan = true;
  lastScanClearTime = millis()+600;
  m_scanStartAt = millis()+500;
  return true;
}

bool BleManager::stopScanning(){
  if (!isScanning){
    return false;
  }
  m_pauseScan = true;
  return true;
}

void BleManager::AddPairedDeviceAddress(std::string addr, BleServiceHandler* obj){
  pairedHandlers[addr] = obj;
}

void BleManager::AddAcceptedService(std::string name, BleServiceHandler* obj){
  handlers[name] = obj;
}

void BleManager::update(){
  if (!m_started){
    return;
  }
  if (millis() - lastScanClearTime >= (30*1000) ) {
    if (isScanning){
      setScanningMode(false);
      if (m_pauseScan){
        Logger::Info("[BLE] Scaner stopped!");
        NimBLEDevice::getScan()->stop();
        m_pauseScan = false;
      }
      NimBLEDevice::getScan()->clearResults(); // Clear the scan results
      Logger::Info("[BLE] Scan results cleared");
      m_canScan = true;
      m_scanStartAt = millis()+1000;
    }
    lastScanClearTime = millis(); // Reset the timer
    Devices::CalculateMemmoryUsage();
  }
  if (m_pauseScan){
    NimBLEDevice::getScan()->stop();
    m_pauseScan = false;
  }
  
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  bool hasConnection = toConnect.ready;
  xSemaphoreGive(m_mutex);
  if (hasConnection) {
    connectToServer();
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    toConnect.erase();
    xSemaphoreGive(m_mutex);
    m_canScan = true;
    m_scanStartAt = millis()+1000;
    return;
  }

  if (m_canScan){ 
    if (clientCount < maxClients){
      if (!isScanning && !hasConnection && m_scanStartAt < millis()){
        setScanningMode(true);
      }
    }
      
    if (clientCount > 0){
      std::string toErase;
      xSemaphoreTake(m_mutex, portMAX_DELAY);
      for (auto &aux : clients){
        if (!aux.second->connected){
            toErase = aux.first;
            break;
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
#endif