
#include "bluetooth/characteristicshandler.hpp"
#include "lua/luainterface.hpp"
#include "bluetooth/ble_client.hpp"

void BleCharacteristicsHandler::AddMessage(int id, uint8_t* pData, size_t length, bool isNotify){
    xSemaphoreTake(queueMutex, portTICK_PERIOD_MS * 50);
    if (dataQueue.size() > 100) {
        dataQueue.pop();
    }
    dataQueue.emplace(id, pData, length);
    xSemaphoreGive(queueMutex);
}

void BleCharacteristicsHandler::SendMessages(){
    while (!dataQueue.empty()){
        std::vector<uint8_t> vec;
        int id = 0;
        xSemaphoreTake(queueMutex, portMAX_DELAY);
        vec = dataQueue.front().message;
        id = dataQueue.front().m_id;
        dataQueue.pop();
        xSemaphoreGive(queueMutex);
        if (luaCallback != nullptr){
            luaCallback->callLuaFunction(id, vec);
        }
    }
}