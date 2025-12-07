#include "tools/logger.hpp"
#include "tools/oledscreen.hpp"
#include <SD.h>

std::string Logger::m_filename = "";
char *Logger::buffer = nullptr;
SemaphoreHandle_t Logger::mutex;
File Logger::logFile;
bool Logger::writingLog;
bool Logger::started;

void Logger::Allocate(){
  Logger::buffer = (char*)ps_malloc(sizeof(char)*1024);
}

void Logger::Begin(){
  

    Logger::mutex = xSemaphoreCreateMutex();
    Logger::writingLog = false;
    int sequence = 0;
    File myFile = SD.open("/cache/sequence.txt", FILE_READ);
    if (!myFile){
      myFile = SD.open("/cache/sequence.txt", FILE_WRITE);
      myFile.print("0");
      myFile.close();
    }else{
      int cnt = 0;
      while (myFile.available()) {
        buffer[cnt] = myFile.read();
        Serial.print(buffer[cnt]);
        cnt++;
      }
      buffer[cnt] = 0;
      sequence = atoi(buffer);
      myFile.close();
      myFile = SD.open("/cache/sequence.txt", FILE_WRITE);
      myFile.seek(0);
      sprintf(buffer, "%d", sequence+1);
      myFile.print(buffer);
      myFile.close();
    }
      
    SD.mkdir("/logs");
    sprintf(buffer, "/logs/log_%d.txt", sequence);
    m_filename = buffer;

    started = true;

}

void Logger::WriteLogHead(char mode){
    int now = millis();
    Serial.print("[");
    Serial.print(now);
    Serial.printf("][%c] ", mode);

}

void Logger::log(const char *c){
  Serial.print(c);
  if (Logger::started){
    xSemaphoreTake(Logger::mutex, portMAX_DELAY);
    if (!Logger::writingLog){
        Logger::logFile = SD.open(m_filename.c_str(), FILE_APPEND);
        Logger::writingLog = true;
    }
    if (Logger::logFile){
        Logger::logFile.print(c);
    }
    xSemaphoreGive(Logger::mutex);
  }
}

void Logger::logln(const char *c){
  Serial.println(c);
  if (Logger::started){
    xSemaphoreTake(Logger::mutex, portMAX_DELAY);
    if (!Logger::writingLog){
        Logger::logFile = SD.open(m_filename.c_str(), FILE_APPEND);
        Logger::writingLog = true;
    }
    if (Logger::logFile){
        Logger::logFile.println(c);
        Logger::writingLog = false;
        Logger::logFile.close();
    }
    xSemaphoreGive(Logger::mutex);
  }
  if (OledScreen::consoleMode){
    OledScreen::PrintConsole(c);
  }
  
}

void Logger::log(std::string c){
  const char *cstr = c.c_str();
  log(cstr);
}

void Logger::logln(std::string c){
  const char *cstr = c.c_str();
  logln(cstr);
}