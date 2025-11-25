
#include "bluetooth/characteristicshandler.hpp"
#include "lua/luainterface.hpp"
#include "bluetooth/ble_client.hpp"



void BleCharacteristicsHandler::AddMessage(uint8_t* pData, size_t length, bool isNotify){
    xSemaphoreTake(queueMutex, portTICK_PERIOD_MS * 50);
    if (dataQueue.size() > 100) {
        dataQueue.pop();
    }
    dataQueue.emplace(pData, length);
    xSemaphoreGive(queueMutex);
}


void BleCharacteristicsHandler::SendMessages(){
    luaCallback->callLuaFunction("pica");
    while (!dataQueue.empty()){
        std::vector<uint8_t> vec;
        xSemaphoreTake(queueMutex, portMAX_DELAY);
        vec = dataQueue.front().message;
        dataQueue.pop();
        xSemaphoreGive(queueMutex);
        if (luaCallback != nullptr){
            luaCallback->callLuaFunction(vec);
        }
        //g_lua.CallFunctionT("onBluetoothCallback", vec);
  }
}