#include "bluetooth/characteristicshandler.hpp"
#ifdef ENABLE_BLE
#include "lua/luainterface.hpp"
#include "bluetooth/ble_client.hpp"

void BleCharacteristicsHandler::AddMessage(int cliID, int id, uint8_t* pData, size_t length, bool isNotify, int handle){
    if (xSemaphoreTake(queueMutex, portTICK_PERIOD_MS * 50) != pdTRUE){
        return;
    }
    if (dataQueue.size() > 100) {
        dataQueue.pop();
    }
    dataQueue.emplace(cliID, id, pData, length, handle);
    xSemaphoreGive(queueMutex);
}

void BleCharacteristicsHandler::processMessageAndPopAndUnlock(){
    PSRAMVector<uint8_t> vec;
    int id = -1;
    int cliId = -1;
    int handle = -1;
    

    vec = dataQueue.front().message;
    id = dataQueue.front().m_id;
    cliId = dataQueue.front().m_CliId;
    handle = dataQueue.front().m_handle;
    dataQueue.pop();

    xSemaphoreGive(queueMutex);

    #ifdef ENABLE_LUA
    if (luaCallback != nullptr){
        luaCallback->callLuaFunction(id, cliId, vec, handle);
    }
    #endif
}

bool BleCharacteristicsHandler::GetMessageAndSend(){
    if (xSemaphoreTake(queueMutex, portTICK_PERIOD_MS * 25) != pdTRUE){
        return false;
    }
    if (dataQueue.empty()){
        xSemaphoreGive(queueMutex);
        return false;
    }

    processMessageAndPopAndUnlock();
    return true;
}

void BleCharacteristicsHandler::SendMessages(){
    if (m_stream){
        while (GetMessageAndSend()){}
    }else{
        GetMessageAndSend();
    }
}
#endif