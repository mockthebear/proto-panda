#include "bluetooth/servicehandler.hpp"
#include "lua/luainterface.hpp"
#include "bluetooth/ble_client.hpp"


BluetoothDeviceHandler::~BluetoothDeviceHandler(){
    m_callbacks = nullptr;
}

BleCharacteristicsHandler* BleServiceHandler::AddCharacteristics(std::string uuid){
    NimBLEUUID charId(uuid);

    if (charId == NimBLEUUID()){
        Logger::Error("uuid provided '%s' is invalid.", uuid.c_str());
        return nullptr;
    } 

    charId = charId.to128();

    auto obj = new BleCharacteristicsHandler(charId);
    m_characteristics[charId.toString()] = obj;
    return obj;
}


/*void BleServiceHandler::AddMessage(const NimBLEUUID &charId,uint8_t* pData, size_t length, bool isNotify){
    std::string charName = charId.toString();
    BleCharacteristicsHandler* characteristic = m_characteristics[charName];
    if (characteristic){
        characteristic->AddMessage(pData, length, isNotify);
    }else{
        if (warnedMap[charName] == false){
            Logger::Error("Message arrived on unmapped characteristics %s ", charName.c_str());
            warnedMap[charName] = true;
        }
    }  
}*/

void BleServiceHandler::AddDevice(BluetoothDeviceHandler *dev){
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    devicesToNotify.push(dev);
    xSemaphoreGive(queueMutex);
}

bool BleServiceHandler::WriteToCharacteristics(std::vector<uint8_t> bytes, int clientId, std::string charName, bool reply){
    NimBLEUUID charId(charName);

    if (charId == NimBLEUUID()){
        Logger::Error("uuid provided '%s' is invalid.", charName.c_str());
        return false;
    } 

    if (clientId < 0 || clientId > m_connectedDevices.size()){
        Logger::Error("id is not present!");
        return false;
    }
    BluetoothDeviceHandler *dev = m_connectedDevices[clientId-1];
    if (dev == nullptr){
        Logger::Error("device is not found");
        return false;
    }

    if (dev->m_client == nullptr){
        Logger::Error("device dont seems to have a valid client");
        return false;
    }

    NimBLERemoteService* svc = dev->m_client->getService(uuid);
    if (svc == nullptr){
        Logger::Error("For some reason. The device lost the service?!");
        return false;
    }
    NimBLERemoteCharacteristic* chr = svc->getCharacteristic(charId);
    if (svc == nullptr){
        Logger::Error("Service '%s' does not have characteristics '%s'", uuid.toString().c_str(), charId.toString().c_str());
        return false;
    } 
    uint8_t *bytesData = bytes.data();
    bool res = chr->writeValue(bytesData, sizeof(uint8_t) * bytes.size(), true);
    return res;
}

void BleServiceHandler::SendMessages(){
    if (devicesToNotify.size() > 0){
        xSemaphoreTake(queueMutex, portMAX_DELAY);
        BluetoothDeviceHandler *dev = devicesToNotify.top();
        m_connectedDevices.emplace_back(dev);
        int id = m_connectedDevices.size();
        devicesToNotify.pop();
        xSemaphoreGive(queueMutex);
        if (luaOnConnectCallback != nullptr){
            luaOnConnectCallback->callLuaFunction(id, dev->m_device->getAddress().toString(), dev->m_device->getName().c_str());
        }
    }
    for (auto &it : m_characteristics){
        it.second->SendMessages();
    }  
    
}

std::vector<BleCharacteristicsHandler*> BleServiceHandler::getCharacteristics(){
    std::vector<BleCharacteristicsHandler*> list;
    for (auto &it : m_characteristics){
        list.emplace_back(it.second);
    }  
    return list;
}
