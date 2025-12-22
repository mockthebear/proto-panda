#pragma once

#include "config.hpp"
#include "tools/storage.hpp"

#include <Arduino.h>

#include <string.h>

class Logger{
    public:
        static void Allocate();
        static void Begin();
        static void log(std::string c);
        static void log(const char *c);
        static void logln(const char *c);
        static void logln(std::string c);

        template<typename... arg> static void Info(const char *c, const arg&... a );
        template<typename... arg> static void Error(const char *c, const arg&... a );
        template<typename... arg> static void Print(const char *c, const arg&... a );
        template<typename... arg> static void Println(const char *c, const arg&... a );
    private:
        
        static void WriteLogHead(char c);
        static File logFile;
        static bool writingLog;
        static bool started;
        static std::string m_filename;
        static SemaphoreHandle_t mutex;
        static char *buffer;
};

template<typename... arg> void Logger::Info(const char *c, const arg&... a )
{
    sprintf (buffer, c, a...);
    Logger::WriteLogHead('I');
    Logger::logln(buffer);
}


template<typename... arg> void Logger::Error(const char *c, const arg&... a )
{
    sprintf (buffer, c, a...);
    Logger::WriteLogHead('E');
    Logger::logln(buffer);
}

template<typename... arg> void Logger::Print(const char *c, const arg&... a )
{
    sprintf (buffer, c, a...);
    Logger::log(buffer);
}

template<typename... arg> void Logger::Println(const char *c, const arg&... a )
{
    sprintf (buffer, c, a...);
    Logger::logln(buffer);
}

