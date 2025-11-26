#include "bluetooth/servicehandler.hpp"
#include "lua/luainterface.hpp"
#include "bluetooth/ble_client.hpp"


BluetoothDeviceHandler::~BluetoothDeviceHandler(){
    m_callbacks = nullptr;
}

BleCharacteristicsHandler* BleServiceHandler::AddCharacteristics(std::string uuid){
    if (uuid.length() != 4 ) {
        return nullptr;
    }

    NimBLEUUID charId(uuid);
    auto obj = new BleCharacteristicsHandler(charId);
    m_characteristics[charId.to16().toString()] = obj;
    return obj;
}

void BleServiceHandler::AddMessage(const NimBLEUUID &charId,uint8_t* pData, size_t length, bool isNotify){
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
}

void BleServiceHandler::SendMessages(){
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
