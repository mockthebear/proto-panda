#include "tools/ir.hpp"
#include "bluetooth/ble_client.hpp"
#include "tools/logger.hpp"

void IRAM_ATTR infraredInterrupt() {
  g_InfraRed.CalculateDifference();
}

InfraRedManager::InfraRedManager(){
    m_started = false;
    m_interruptPin = 0;
    m_lastTrigger = 0;
    m_counter = 0;
}

void InfraRedManager::update(){
    if (!m_started){
        return;
    }
    if (m_counter > 0){
        if (micros()-m_lastTrigger > 250000){
            decodeIR();
            m_counter = 0;
        }
    }
}

IrCommand InfraRedManager::getLastCommand(){
    if (hasCommand()){
        return IrCommand();
    }
    IrCommand aux = m_commands.top();
    m_commands.pop();
    return aux;
}

void InfraRedManager::decodeIR() {
    if (m_counter < 68) {
        return;
    }
    
    uint32_t data = 0;
    bool validFrame = true;
    
    int startIdx = -1;
    //Find the LONG pulse at the begin
    for (int i = 0; i < m_counter - 1; i++) {
        if (m_steps[i] > 8000 && m_steps[i] < 10000) {
            if (m_steps[i+1] > 4000 && m_steps[i+1] < 5000) {
                startIdx = i;
                break;
            }
        }
    }
    
    if (startIdx == -1) {
        return;
    }
    
    int dataStartIdx = startIdx + 2;

    for (int i = 0; i < 32; i++) {
        int markIdx = dataStartIdx + (i * 2);
        int spaceIdx = markIdx + 1;
        
        if (spaceIdx >= m_counter) {
            return;
        }
        
        // about ~560µs
        uint32_t mark = m_steps[markIdx];
        if (mark < 400 || mark > 700) {
            validFrame = false;
            break;
        }
        
        uint32_t space = m_steps[spaceIdx];
        if (space > 1000 && space < 2000) {
            data |= (1UL << i); 
        } else if (space < 400 || space > 800) {
            validFrame = false;
            break;
        }
    }
    
    if (validFrame) {
        uint16_t address = data & 0xFFFF;
        uint16_t command = (data >> 16) & 0xFFFF;
        uint8_t cmd = command & 0xFF;
        uint8_t invCmd = (command >> 8) & 0xFF;
        if (cmd == (uint8_t)(~invCmd)) {
            m_commands.push(IrCommand(address, invCmd));   
        }

             
    }
}

void InfraRedManager::CalculateDifference() {
    uint32_t now = micros();
    uint32_t diff = now - m_lastTrigger;
    m_lastTrigger = now;
    m_steps[m_counter] = diff;
    m_counter++;
    if (m_counter > 255){
        m_counter = 0;
    }
}

bool InfraRedManager::begin(){
    if (g_remoteControls.IsStarted()){
        Logger::Info("[IR] Cannot start IR while BLE is active");
        return false;
    }
    Logger::Info("[IR] Started IR");
    m_started = true;
    m_steps = new uint32_t[255]; //need to be on ram. We cant afford psram here rn.
    memset(m_steps, 0, sizeof(uint32_t)*255);
    return true;
}
void InfraRedManager::setInterruptPin(uint16_t p){
    if (m_interruptPin != 0){
        disableInterrupt();
    }
    pinMode(p, INPUT);
    delay(1);
    m_interruptPin = digitalPinToInterrupt(p);
}


void InfraRedManager::enableInterrupt(int mode){
    attachInterrupt(m_interruptPin, infraredInterrupt, CHANGE);
    m_interruptMode = mode;
}
void InfraRedManager::disableInterrupt(){
    detachInterrupt(m_interruptPin);
}