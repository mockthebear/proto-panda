#include "bluetooth/servicehandler.hpp"
#include "lua/luainterface.hpp"
#include "bluetooth/ble_client.hpp"


BluetoothDeviceHandler::~BluetoothDeviceHandler(){
    m_callbacks = nullptr;
}

void BleServiceHandler::AddCharacteristics(NimBLEUUID charId){
    m_characteristics[charId.to16().toString()] = new BleCharacteristicsHandler(charId);
}

void BleServiceHandler::AddCharacteristics_TMP(NimBLEUUID charId){
    m_characteristics[charId.to16().toString()] = new BleCharacteristicsHandler(charId, true, false, true);
}

void BleServiceHandler::AddMessage(const NimBLEUUID &charId,uint8_t* pData, size_t length, bool isNotify){
    BleCharacteristicsHandler* characteristic = m_characteristics[charId.toString()];
    if (characteristic){
        characteristic->AddMessage(pData, length, isNotify);
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
