#include "bluetooth/servicehandler.hpp"
#ifdef ENABLE_BLE
#include "lua/luainterface.hpp"
#include "bluetooth/ble_client.hpp"
#include "tools/logger.hpp"


BluetoothDeviceHandler::~BluetoothDeviceHandler(){
    m_callbacks = nullptr;
}


int BluetoothDeviceHandler::idCounter = 0;

BleCharacteristicsHandler* BleServiceHandler::AddCharacteristics(std::string uuid){
    NimBLEUUID charId(uuid);

    if (charId == NimBLEUUID()){
        Logger::Error("uuid provided '%s' is invalid.", uuid.c_str());
        return nullptr;
    } 

    charId = charId.to128();

    BleCharacteristicsHandler *obj = (BleCharacteristicsHandler*)ps_malloc(sizeof(BleCharacteristicsHandler));
    #ifndef __INTELLISENSE__
    //This code will be compiled. But for some reason intellisense claims its invalid. So i'm using this to avoid visual warning in the IDE
    new (obj) BleCharacteristicsHandler(charId);
    #endif

    m_characteristics[charId.toString()] = obj;
    return obj;
}



void BleServiceHandler::AddDevice(BluetoothDeviceHandler *dev){
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    devicesToNotify.push(dev);
    xSemaphoreGive(queueMutex);
}


MultiReturn<std::vector<uint8_t>> BleServiceHandler::ReadFromCharacteristics(int clientId, std::string charName){
    NimBLEUUID charId(charName);

    if (charId == NimBLEUUID()){
        return MultiReturn<std::vector<uint8_t>>("UUID is invalid");
    } 

    BluetoothDeviceHandler *dev = nullptr;
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    for (auto &it : m_connectedDevices){
        if (it->getId() == clientId){
            dev = it;
            break;
        }
    }
    xSemaphoreGive(queueMutex);

    if (dev == nullptr){
        return MultiReturn<std::vector<uint8_t>>("device is not found");
    }

    if (dev->m_client == nullptr){
        return MultiReturn<std::vector<uint8_t>>("device dont seems to have a valid client");
    }

    NimBLERemoteService* svc = dev->m_client->getService(uuid);
    if (svc == nullptr){
        return MultiReturn<std::vector<uint8_t>>("For some reason. The device lost the service?!");
    }
    NimBLERemoteCharacteristic* chr = svc->getCharacteristic(charId);
    if (svc == nullptr){
        return MultiReturn<std::vector<uint8_t>>("Service does not have the characteristics");
    } 
    NimBLEAttValue val = chr->readValue();
    if (val.size() == 0){
        return MultiReturn<std::vector<uint8_t>>("Not read");
    }
    return MultiReturn<std::vector<uint8_t>>(true, std::vector<uint8_t>(val.data(), val.data()+val.size()));
}

MultiReturn<std::vector<uint8_t>> BleServiceHandler::ReadDescriptor(int clientId, int charHandle, std::string descName){
    NimBLEUUID descId(descName);

    if (descId == NimBLEUUID()){
        return MultiReturn<std::vector<uint8_t>>("UUID is invalid");
    }

    BluetoothDeviceHandler *dev = nullptr;
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    for (auto &it : m_connectedDevices){
        if (it->getId() == clientId){
            dev = it;
            break;
        }
    }
    xSemaphoreGive(queueMutex);

    if (dev == nullptr){
        return MultiReturn<std::vector<uint8_t>>("device is not found");
    }

    if (dev->m_client == nullptr){
        return MultiReturn<std::vector<uint8_t>>("device dont seems to have a valid client");
    }

    NimBLERemoteService* svc = dev->m_client->getService(uuid);
    if (svc == nullptr){
        return MultiReturn<std::vector<uint8_t>>("For some reason. The device lost the service?!");
    }

    NimBLERemoteCharacteristic* chr = nullptr;
    for (auto *it : svc->getCharacteristics(false)){
        if (it->getHandle() == charHandle){
            chr = it;
            break;
        }
    }
    if (chr == nullptr){
        return MultiReturn<std::vector<uint8_t>>("Service does not have a characteristic with that handle");
    }

    NimBLERemoteDescriptor* desc = chr->getDescriptor(descId);
    if (desc == nullptr){
        return MultiReturn<std::vector<uint8_t>>("Characteristic does not have that descriptor");
    }

    NimBLEAttValue val = desc->readValue();
    if (val.size() == 0){
        return MultiReturn<std::vector<uint8_t>>("Not read");
    }
    return MultiReturn<std::vector<uint8_t>>(true, std::vector<uint8_t>(val.data(), val.data()+val.size()));
}

int BleServiceHandler::GetClientIdFromControllerId(uint32_t id){
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    for (auto &it : m_connectedDevices){
        if (id == it->m_controllerId){
            xSemaphoreGive(queueMutex);
            return it->getId();
        }
    }
    xSemaphoreGive(queueMutex);
    return -1;
}

MultiReturn<int> BleServiceHandler::GetRSSI(int clientId){
    BluetoothDeviceHandler *dev = nullptr;
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    for (auto &it : m_connectedDevices){
        if (it->getId() == clientId){
            dev = it;
            break;
        }
    }
    xSemaphoreGive(queueMutex);

    if (dev == nullptr){
        return MultiReturn<int>("device is not found");
    }

    if (dev->m_client == nullptr){
        return MultiReturn<int>("device dont seems to have a valid client");
    }

    return MultiReturn<int>(true, dev->m_client->getRssi());
}

std::vector<BleCharacteristicsHandler*> BleServiceHandler::getRegisteredCharacteristics(){
    std::vector<BleCharacteristicsHandler*> list;
    for (auto &it : m_characteristics){
        list.emplace_back(it.second);
    }  
    return list;
}


MultiReturn<std::vector<std::string>> BleServiceHandler::GetCharacteristicsFromOurService(int clientId){
    BluetoothDeviceHandler *dev = nullptr;
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    for (auto &it : m_connectedDevices){
        if (it->getId() == clientId){
            dev = it;
            break;
        }
    }
    xSemaphoreGive(queueMutex);

    if (dev == nullptr){
        return MultiReturn<std::vector<std::string>>("device is not found");
    }

    if (dev->m_client == nullptr){
        return MultiReturn<std::vector<std::string>>("device dont seems to have a valid client");
    }

    NimBLERemoteService* svc = dev->m_client->getService(uuid);
    if (svc == nullptr){
        return MultiReturn<std::vector<std::string>>("For some reason. The device lost the service?!");
    }
    auto chr = svc->getCharacteristics(false);
    std::vector<std::string> resp;
    for (auto &it : chr){
        const char *c = it->getUUID().toString().c_str();
        resp.emplace_back(c, c+strlen(c));
    }
    return MultiReturn<std::vector<std::string>>(true, resp);
}

MultiReturn<std::vector<std::string>> BleServiceHandler::GetServices(int clientId, bool refresh){
    
    BluetoothDeviceHandler *dev = nullptr;
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    for (auto &it : m_connectedDevices){
        if (it->getId() == clientId){
            dev = it;
            break;
        }
    }
    xSemaphoreGive(queueMutex);

    if (dev == nullptr){
        return MultiReturn<std::vector<std::string>>("device is not found");
    }

    if (dev->m_client == nullptr){
        return MultiReturn<std::vector<std::string>>("device dont seems to have a valid client");
    }

    auto chr = dev->m_client->getServices(refresh);
    std::vector<std::string> resp;
    for (auto &it : chr){
        NimBLEUUID copyUUID = it->getUUID();
        std::string str = copyUUID.to128();
        resp.emplace_back(str);
    }
    return MultiReturn<std::vector<std::string>>(true, resp);
}

MultiReturn<std::vector<std::string>> BleServiceHandler::GetCharacteristicsFromService(int clientId, std::string servName, bool refresh){
     NimBLEUUID charId(servName);

    if (charId == NimBLEUUID()){
        return MultiReturn<std::vector<std::string>>("UUID provided is invalid");
    } 

    BluetoothDeviceHandler *dev = g_remoteControls.getDeviceById(clientId);
    if (dev == nullptr){
        return MultiReturn<std::vector<std::string>>("device is not found");
    }

    if (dev->m_client == nullptr){
        return MultiReturn<std::vector<std::string>>("device dont seems to have a valid client");
    }
    NimBLERemoteService* svc = dev->m_client->getService(charId);
    if (svc == nullptr){
        return MultiReturn<std::vector<std::string>>("Service is not avaliable");
    }

    auto chr = svc->getCharacteristics(refresh);
    std::vector<std::string> resp;
    for (auto &it : chr){
        const char *c = it->getUUID().toString().c_str();
        resp.emplace_back(c, c+strlen(c));
    }
    return MultiReturn<std::vector<std::string>>(true, resp);

}

void BleServiceHandler::AddPairedDeviceAddress(std::string addr){
    g_remoteControls.AddPairedDeviceAddress(addr, this);
    Logger::Info("Accepting paired device addr %s", addr);
}

bool BleServiceHandler::WriteToCharacteristics(std::vector<uint8_t> bytes, int clientId, std::string charName, bool reply){
    NimBLEUUID charId(charName);

    if (charId == NimBLEUUID()){
        Logger::Error("uuid provided '%s' is invalid.", charName.c_str());
        return false;
    }  

    BluetoothDeviceHandler *dev = nullptr;
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    for (auto &it : m_connectedDevices){
        if (it->getId() == clientId){
            dev = it;
            break;
        }
    }
    xSemaphoreGive(queueMutex);
    

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
        if (devicesToNotify.size() == 0){
            xSemaphoreGive(queueMutex);
            return;
        }
        BluetoothDeviceHandler *dev = devicesToNotify.top();
        m_connectedDevices.emplace_back(dev);
        devicesToNotify.pop();
        xSemaphoreGive(queueMutex);
        #ifdef ENABLE_LUA
        if (luaOnConnectCallback != nullptr){
            luaOnConnectCallback->callLuaFunction(dev->getId(), dev->m_controllerId, dev->m_deviceAddress, dev->m_deviceName);
        }
        #endif
    }
    if (devicesToDisconnectNotify.size() > 0){
        xSemaphoreTake(queueMutex, portMAX_DELAY);
        auto msg = devicesToDisconnectNotify.top();
        devicesToDisconnectNotify.pop();
        xSemaphoreGive(queueMutex);
        #ifdef ENABLE_LUA
        if (luaOnDisconnectCallback != nullptr){
            luaOnDisconnectCallback->callLuaFunction(msg.id, msg.controllerId, msg.reason);
        }
        #endif
    }
    for (auto &it : m_characteristics){
        it.second->SendMessages();
    }  
    
}


void BleServiceHandler::NotifyDisconnect(int conId, int clientId, const char* reason){
    DisconnectTuple tp;
    tp.id = conId;
    tp.controllerId = clientId;
    tp.reason = reason;
    xSemaphoreTake(queueMutex, portMAX_DELAY);
    devicesToDisconnectNotify.push(tp);
    g_remoteControls.requestClearResults();
    xSemaphoreGive(queueMutex);
}
#endif