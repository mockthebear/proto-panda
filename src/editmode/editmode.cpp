#include "editmode/editmode.hpp"

#include "tools/logger.hpp"
#include "drawing/framerepository.hpp"
#include "drawing/icons/icons.hpp"
#include "tools/oledscreen.hpp"
#include "tools/storage.hpp"
#include "lua/luainterface.hpp"
#include "tools/devices.hpp"
#include <Arduino.h>

#include <WiFi.h>
#include <ArduinoJson.h>
#include "SD.h"

#include "editmode/ftp/FtpServer.h"

FtpServer *ftpSrv;
WiFiServer *luaServer;



extern FrameRepository g_frameRepo;
extern LuaInterface g_lua;

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace)
{
  switch (ftpOperation)
  {
  case FTP_CONNECT:
    Serial.println(F("FTP: Connected!"));
    break;
  case FTP_DISCONNECT:
    Serial.println(F("FTP: Disconnected!"));
    break;
  case FTP_FREE_SPACE_CHANGE:
    Serial.printf("FTP: Free space change, free %u of %u!\n", freeSpace, totalSpace);
    break;
  default:
    break;
  }
};
void _transferCallback(FtpTransferOperation ftpOperation, const char *name, unsigned int transferredSize)
{
  switch (ftpOperation)
  {
  case FTP_UPLOAD_START:
    Serial.println(F("FTP: Upload start!"));
    break;
  case FTP_UPLOAD:
    Serial.printf("FTP: Upload of file %s byte %u\n", name, transferredSize);
    break;
  case FTP_TRANSFER_STOP:
    Serial.println(F("FTP: Finish transfer!"));
    break;
  case FTP_TRANSFER_ERROR:
    Serial.println(F("FTP: Transfer error!"));
    break;
  default:
    break;
  }
};

void EditMode::CheckBeginEditMode(){
  OledScreen::display.clearDisplay();
  OledScreen::display.drawBitmap(0,0, icon_wifi, 128, 64, 1);
  OledScreen::display.display();
  delay(1000);


  OledScreen::SetConsoleMode(true);
  
  if (digitalRead(EDIT_MODE_PIN) == 0)
  {
    for (int i = 0; i < 10; i++)
    {
      OledScreen::display.clearDisplay();
      if (i%2 == 0){
        OledScreen::display.drawBitmap(0,0, icon_release_1, 128, 64, 1);
      }else{
        OledScreen::display.drawBitmap(0,0, icon_release_2, 128, 64, 1);
      }
      
      OledScreen::display.display();
      delay(200);
      if (digitalRead(EDIT_MODE_PIN) == 1)
      {
        DoBegin(false);
        return;
      }
    }
    DoBegin(true);
  }
}

void EditMode::DoBegin(bool useSSID)
{
  if (!Storage::Begin()){
    OledScreen::display.clearDisplay();
    OledScreen::display.drawBitmap(0,0, icon_sd, 128, 64, 1);
    OledScreen::display.display();
    for (;;){}
  }
  Logger::Begin();

  if (!g_frameRepo.Begin()){
    OledScreen::CriticalFail("Frame repository has failed! If restarting does not solve, its a hardware problem.");
    for (;;){
      Devices::BuzzerTone(420);
      delay(200);
      Devices::BuzzerTone(420);
      delay(200);
      Devices::BuzzerNoTone();
      delay(1000);
    }
  }

  if (useSSID)
  {
    File file = SD.open("/SSID.json", "r");
    if (!file)
    {
      OledScreen::CriticalFail("Failed\nto open\n/SSID.json");
      for (;;)
      {
      }
    }

    SpiRamAllocator allocator;
    JsonDocument json_doc(&allocator);
    auto err = deserializeJson(json_doc, file);
    if (err)
    {
      OledScreen::CriticalFail(err.c_str());
      for (;;)
      {
      }
    }

    if (!json_doc.containsKey("name") || !json_doc["name"].is<const char *>())
    {
      OledScreen::CriticalFail("Expected\nfield\n'name'");
      for (;;)
      {
      }
    }
    if (!json_doc.containsKey("password") || !json_doc["password"].is<const char *>())
    {
      OledScreen::CriticalFail("Expected\nfield\n'password'");
      for (;;)
      {
      }
    }

    const char *name = json_doc["name"];
    const char *password = json_doc["password"];

    WiFi.begin(name, password);
    Logger::Info("Connecting to %s", name);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Logger::Info("Trying...");
    }
    Logger::Info("Connected!");
    if (EDIT_MODE_HTTP_PORT == 80){
      Logger::Info("http://%s", WiFi.localIP().toString().c_str());
    }else{
      Logger::Info("http://%s:%d", WiFi.localIP().toString().c_str(), EDIT_MODE_HTTP_PORT);
    }
    Logger::Info("FTP: %s:%d", WiFi.localIP().toString().c_str(), EDIT_MODE_FTP_PORT);
  }
  else
  {
    WiFi.softAP(WIFI_AP_NAME, WIFI_AP_PASSWORD);
    Logger::Info("Network: %s", WIFI_AP_NAME);
    Logger::Info("Pass: %s", WIFI_AP_PASSWORD);
    Logger::Info("FTP: %s:%d", WiFi.softAPIP().toString().c_str(), EDIT_MODE_FTP_PORT);
  }

  ftpSrv = new FtpServer(EDIT_MODE_FTP_PORT);
  luaServer = new WiFiServer(EDIT_MODE_LUA_PORT);
  

  ftpSrv->setCallback(_callback);
  ftpSrv->setTransferCallback(_transferCallback);

  Logger::Info("User: %s", EDIT_MODE_FTP_USER);
  Logger::Info("Password: %s",  EDIT_MODE_FTP_PASSWORD);
  ftpSrv->begin(EDIT_MODE_FTP_USER, EDIT_MODE_FTP_PASSWORD);
  m_running = true;

  if (!g_lua.Start()){
    OledScreen::CriticalFail("Failed to initialize Lua!");
    return;
  }
  Logger::Info("Lua on port %d", EDIT_MODE_LUA_PORT);
  LuaInterface::HaltIfError = false;
  luaServer->begin();  
  startWifiServer();
}

bool EditMode::IsOnEditMode(){
  return m_running;
}

void handleClient(WiFiClient &client){
  client.println("Type the lua script!\n");
  while(client.connected()) {
    if (client.available()) {
      String request = client.readStringUntil('\r');
      if (request == "exit"){
        client.stop();
        return;
      }

      client.printf("Running: %s\n", request.c_str());
      if (!g_lua.DoString(request.c_str())){
        client.printf("> Error: \n");
        client.println(g_lua.getLastError());
      }else{
        client.printf("> Done\n");
      }

    }
    ftpSrv->handleFTP();
    delay(1);
  }
}


void EditMode::LoopEditMode(){
  ftpSrv->handleFTP();

  WiFiClient client = luaServer->available();
  if (client){
    handleClient(client);
  }
}