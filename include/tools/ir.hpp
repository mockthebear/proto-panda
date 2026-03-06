#pragma once
#include "config.hpp"
#include <stdint.h>
#include <Arduino.h>
#include <stack>

class IrCommand{
    public:
        IrCommand():address(0),command(0){};
        IrCommand(uint16_t addr, uint16_t cmd):address(addr),command(cmd){};
        uint16_t address;
        uint16_t command;
};

class InfraRedManager{
    public:
        InfraRedManager();

        inline uint16_t getInterruptPin(){
            return m_interruptPin;
        };

        bool begin();
        inline bool IsStarted(){ return m_started; };
        inline void CalculateDifference();
        void setInterruptPin(uint16_t p);
        void enableInterrupt(int mode = FALLING);
        void disableInterrupt();

        void update();
        IrCommand getLastCommand();
        inline bool hasCommand(){
            return m_commands.empty();
        }
    private:
        void decodeIR();
        uint16_t m_interruptPin;
        int m_interruptMode;
        uint32_t m_lastTrigger;
        uint32_t *m_steps;
        uint32_t m_counter;
        bool m_started;

        std::stack<IrCommand> m_commands;
};

extern InfraRedManager g_InfraRed;