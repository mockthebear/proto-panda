#include "lua/luainterface.hpp"
#include "lua/luaobject.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include "tools/sensors.hpp"
#include "tools/oledscreen.hpp"
#include "tools/storage.hpp"
#include "drawing/animation.hpp"
#include "drawing/ledstrip.hpp"
#include "bluetooth/ble_client.hpp"
#include "drawing/dma_display.hpp"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include <FFat.h>
#include "lua/luafunctions/drawingfunctions.hpp"


extern LedStrip g_leds;


static PersistentDictionary dict;

bool LuaInterface::HaltIfError = true;
std::string LuaInterface::lastError;

int getInternalButtonStatus()
{
  return digitalRead(EDIT_MODE_PIN);
}

void setTimeoutSerialIo(int tm){
  Serial2.setTimeout(tm);
}
void beginSerialIo(int baud)
{
  Serial2.begin(baud, SERIAL_8N1, 1, 2);
 
}

int serialIoAvaliable()
{
  
  return Serial2.available();
}

int serialAvaliable()
{
  return Serial.available();
}


std::string serialReadStringUntil(char terminator)
{
  String str = Serial.readStringUntil(terminator);
  return std::string(str.c_str());
}
std::string serialIoReadStringUntil(char terminator)
{
  String str = Serial2.readStringUntil(terminator);
  return std::string(str.c_str());
}
int serialIoRead()
{
  return Serial2.read();
}

int serialRead()
{
  return Serial.read();
}

int serialIoWrite(uint8_t data)
{
  return Serial2.write(data);
}

int serialWrite(uint8_t data)
{
  return Serial.write(data);
}

int serialIoAvailableForWrite()
{
  return Serial2.availableForWrite();
}

int serialAvailableForWrite()
{
  return Serial.availableForWrite();
}

int serialIoWriteString(std::string data)
{
  return Serial2.write(data.c_str());
}

int serialWriteString(std::string data)
{
  return Serial.write(data.c_str());
}

int wireAvailable() {
  return Wire.available();
}

bool wireBegin(uint8_t addr) {
  return Wire.begin(addr);
}

void wireFlush() {
  Wire.flush();
}

void wireBeginTransmission(uint8_t addr) {
  Wire.beginTransmission(addr);
}

uint8_t wireEndTransmission(bool sendStop = true) {
  return Wire.endTransmission(sendStop);
}

int wireRead() {
  return Wire.read();
}

std::vector<uint8_t> wireReadBytes(int length) {
  std::vector<uint8_t> buffer(length);
  int bytesRead = Wire.readBytes(buffer.data(), length);
  if (bytesRead != length) {
      buffer.resize(bytesRead);
  }
  return buffer;
}

uint8_t wireRequestFrom(uint16_t address, size_t size, bool sendStop) {
  return Wire.requestFrom(address, size, sendStop);
}

int wirePeek() {
  return Wire.peek();
}

float wireParseFloat() {
  return Wire.parseFloat();
}

int wireParseInt() {
  return Wire.parseInt();
}


void wireSetTimeout(uint32_t timeout) {
  Wire.setTimeout(timeout);
}

uint32_t wireGetTimeout() {
  return Wire.getTimeout();
}

void restart() {
  esp_restart();
}


void setBrownoutDetection(bool enable) {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, enable ? 1 : 0);
}

int getResetReason() {
  return (int)esp_reset_reason();
}

uint16_t readLidar()
{
  return Devices::ReadLidar();
}
bool hasLidar()
{
  return Devices::HasLidar();
}



static int dumpStackToSerial(lua_State *L)
{
  QuickDebug::DumpLua(L);
  return 0;
}

void setHaltOnError(bool halt)
{
  LuaInterface::HaltIfError = halt;
}

void powerOn()
{
  #ifdef PIN_ENABLE_REGULATOR
  digitalWrite(PIN_ENABLE_REGULATOR, HIGH);
  #endif
}

void setPoweringMode(int mode)
{
  Devices::SetPowerMode((PowerMode)mode);
}

std::vector<std::string> listFiles(std::string path, bool recursive) {
  std::vector<std::string> fileList;
  if (path.back() != '/') {
      path += '/';
  }
  File dir = SD.open(path.c_str());
  if (!dir) {
      return fileList;
  }
  if (!dir.isDirectory()) {
      dir.close();
      return fileList;
  }
  File file = dir.openNextFile();
  while (file) {
      std::string fileName = file.name();
      if (fileName == "." || fileName == "..") {
          file.close();
          file = dir.openNextFile();
          continue;
      }
      std::string fullPath = path + fileName;
      if (file.isDirectory()) {
          if (recursive) {
              std::vector<std::string> subDirFiles = listFiles(fullPath, recursive);
              fileList.insert(fileList.end(), subDirFiles.begin(), subDirFiles.end());
          }
      } else {
          fileList.push_back(fullPath);
      }
      file.close();
      file = dir.openNextFile();
  }
  dir.close();
  return fileList;
}

bool moveFile(std::string path, std::string pathtgt){
  return SD.rename(path.c_str(), pathtgt.c_str());
}

bool removeFile(std::string path){
  return SD.remove(path.c_str());
}

bool createDir(std::string path){
  return SD.mkdir(path.c_str());
}

bool fileExists(std::string path){
  return SD.exists(path.c_str());
}

bool formatFFAT(bool full){
  return FFat.format(full);
}




void powerOff()
{
  #ifdef PIN_ENABLE_REGULATOR
  digitalWrite(PIN_ENABLE_REGULATOR, LOW);
  #endif
}

void LuaInterface::luaCallbackError(const char *errMsg, lua_State *L)
{
  Logger::Info("Lua error: %s\n", errMsg);
  lastError = errMsg;

  if (LuaInterface::HaltIfError)
  {
    powerOff();
    OledScreen::PrintError(errMsg);
    for (;;)
    {
    }
  }
}

int getConnectedRemoteControls()
{
  return g_remoteControls.getConnectedClientsCount();
}

bool startBLE()
{
  Logger::Info("Starting BLE");
  if (!g_remoteControls.begin())
  {
    OledScreen::CriticalFail("Failed to initialize BLE!");
    for (;;)
    {
    }
  }
  Devices::CalculateMemmoryUsage();
  return true;
}

bool beginRadio()
{
  g_remoteControls.beginRadio();
  Devices::CalculateMemmoryUsage();
  return true;
}

void setLogDiscoveredBle(bool log)
{
  g_remoteControls.setLogDiscoveredClients(log);
}

int isElementIdConnected(int id)
{
  return g_remoteControls.isElementIdConnected(id);
}
void beginScanning()
{
  g_remoteControls.beginScanning();
}
void setMaximumControls(int id)
{
  g_remoteControls.setMaximumControls(id);
}


SizedArray *decodePng(std::string filename)
{
  int lastRcError = 0;
  size_t x;
  size_t y;
  uint16_t* decodedData = Storage::DecodePNG(filename.c_str(), lastRcError,x, y);
  if (lastRcError != 0){
    OledScreen::CriticalFail("File not found");
    for (;;){}
    return nullptr;
  }
  if (!decodedData){
    OledScreen::CriticalFail("Failed to allocate");
    for (;;){}
  }

  SizedArray *aux = new SizedArray();
  aux->data = decodedData;
  aux->size =  x * y;
  aux->deleteAfterInsertion = heap_caps_free;
  //This is required, as LUA and BLE share the same core, if this operation takes too we need to yield here a bit.
  return aux;
}



void DrawDisplayScreen()
{
  OledScreen::display.display();
  return;
}

void DrawSetCursor(int x, int y)
{
  OledScreen::display.setCursor(x, y);
  return;
}
void DrawTextScreen(std::string str)
{
  OledScreen::display.print(str.c_str());
  return;
}

void DrawRectScreen(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  OledScreen::display.drawRect(x, y, w, h, color);
  return;
  
}
void DrawRectFilledScreen(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  OledScreen::display.fillRect(x,y,w,h, color);
  return;
  
}

void DrawPixelScreen(int16_t x, int16_t y,uint16_t color)
{
  OledScreen::display.drawPixel(x,y, color);
  return;
  
}

void DrawLineScreen(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  OledScreen::display.drawLine(x, y, w, h, color);
  return;
}
void DrawCircleScreen(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  OledScreen::display.drawCircle(x,y,r,color);
  return;
}

void DrawFilledCircleScreen(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  OledScreen::display.fillCircle(x, y, r, color);
  return;
}
void DrawSetFontSize(int16_t x)
{
  OledScreen::display.setTextSize(x);
  return;
}
void DrawSetTextColor(int16_t fg, int16_t bg)
{
  OledScreen::display.setTextColor(fg, bg);
  return;
}

const char *dictget(std::string str){
  return dict.get(str);
}
void dictdel(std::string str){
  return dict.del(str);
}

void dictset(std::string str, const char * val){
  dict.set(str, val);
}

void dictSave(){
  dict.Save();
}

void dictLoad(){
  dict.Load();
}

void dictFormat(){
  dict.Format();
}

void logMessage(const char *msg){
  Logger::Info("[LOG-LUA] %s", msg);
}


void LuaInterface::RegisterMethods()
{
  m_lua->FuncRegister("log", logMessage);
  //Dict
  m_lua->FuncRegister("dictGet", dictget);
  m_lua->FuncRegister("dictSet", dictset);
  m_lua->FuncRegister("dictDet", dictdel);
  m_lua->FuncRegister("dictSave", dictSave);
  m_lua->FuncRegister("dictLoad", dictLoad);
  m_lua->FuncRegister("dictFormat", dictFormat);
  //Oleed display internal
  #ifdef ENABLE_HUB75_PANEL
  m_lua->FuncRegister("oledFaceToScreen", DrawPanelFaceToScreen);
  #endif
  m_lua->FuncRegister("oledCreateIcon", OledScreen::CreateIcon);
  m_lua->FuncRegister("oledDrawIcon", OledScreen::DrawIcon);
  m_lua->FuncRegister("oledDrawBottomBar", OledScreen::DrawBottomBar);
  m_lua->FuncRegister("oledClearScreen", OledScreen::Clear);
  m_lua->FuncRegister("oledDisplay", DrawDisplayScreen);
  m_lua->FuncRegister("oledSetCursor", DrawSetCursor);
  m_lua->FuncRegister("oledSetFontSize", DrawSetFontSize);
  m_lua->FuncRegisterOptional("oledSetTextColor", DrawSetTextColor, BLACK);
  m_lua->FuncRegister("oledDrawText", DrawTextScreen);
  m_lua->FuncRegister("oledDrawRect", DrawRectScreen);
  m_lua->FuncRegister("oledDrawFilledRect", DrawRectFilledScreen);
  m_lua->FuncRegister("oledDrawPixel", DrawPixelScreen);
  m_lua->FuncRegister("oledDrawLine", DrawLineScreen);
  m_lua->FuncRegister("oledDrawCircle", DrawCircleScreen);
  m_lua->FuncRegister("oledDrawFilledCircle", DrawFilledCircleScreen);
  //BLE
  m_lua->FuncRegister("startBLE", startBLE);
  m_lua->FuncRegister("startBLERadio", beginRadio);
  m_lua->FuncRegister("getConnectedRemoteControls", getConnectedRemoteControls);
  m_lua->FuncRegister("isElementIdConnected", isElementIdConnected);
  m_lua->FuncRegister("beginBleScanning", beginScanning);
  m_lua->FuncRegister("setMaximumControls", setMaximumControls);
  m_lua->FuncRegister("setLogDiscoveredBleDevices", setLogDiscoveredBle);
  //System
  m_lua->FuncRegister("panelPowerOn", powerOn);
  m_lua->FuncRegister("panelPowerOff", powerOff);
  m_lua->FuncRegister("setPoweringMode", setPoweringMode);
  m_lua->FuncRegisterOptional("waitForPower", Devices::WaitForPower, 64);
  m_lua->FuncRegister("setAutoCheckPowerLevel", Devices::SetAutoCheckPowerLevel);
  m_lua->FuncRegister("setVoltageStopThreshold", Devices::SetVoltageStopThreshold);
  m_lua->FuncRegister("setVoltageStartThreshold", Devices::SetVoltageStartThreshold);
  m_lua->FuncRegister("getBatteryVoltage", Sensors::GetBatteryVoltage);
  m_lua->FuncRegister("getAvgBatteryVoltage", Sensors::GetAvgBatteryVoltage);
  m_lua->FuncRegister("setHaltOnError", setHaltOnError);
  m_lua->FuncRegister("getFps", Devices::getFps); 
  m_lua->FuncRegister("getFreePsram", Devices::getFreePsram); 
  m_lua->FuncRegister("getFreeHeap", Devices::getFreeHeap); 
  #ifdef USE_SERVO
  m_lua->FuncRegister("servoPause", Devices::ServoPause);
  m_lua->FuncRegister("servoResume", Devices::ServoResume); 
  m_lua->FuncRegister("servoMove", Devices::ServoMove);
  #endif
  m_lua->FuncRegister("hasServo", Devices::HasServo);

  //LIDAR
  m_lua->FuncRegister("hasLidar", hasLidar);
  m_lua->FuncRegister("readLidar", readLidar);
  //Internal sensor
  m_lua->FuncRegister("getInternalButtonStatus", getInternalButtonStatus); 
  //Panels
  #ifdef ENABLE_HUB75_PANEL
  m_lua->FuncRegister("startPanels", StartPanels); 
  m_lua->FuncRegister("flipPanelBuffer", FlipScreen);
  m_lua->FuncRegister("drawPanelRect", DrawRect);
  m_lua->FuncRegister("drawPanelFillRect", DrawFillRect);
  m_lua->FuncRegister("drawPanelPixel", DrawPixel);
  m_lua->FuncRegisterOptional("drawPanelChar", DrawChar, 1, 0, color565(255,255,255));
  m_lua->FuncRegister("drawPanelPixels", DrawPixels);
  m_lua->FuncRegister("drawPanelLine", DrawLine);
  m_lua->FuncRegister("drawPanelCircle", DrawCircle);
  m_lua->FuncRegister("drawPanelFillCircle", DrawFillCircle);
  m_lua->FuncRegister("clearPanelBuffer", ClearScreen);
  m_lua->FuncRegister("drawPanelFace", DrawFace);
  m_lua->FuncRegisterOptional("setPanelAnimation", setAnimation, -1, false, -1, 250);
  m_lua->FuncRegister("popPanelAnimation", popPanelAnimation); 
  m_lua->FuncRegister("setPanelColorMode", setColorMode); 
  m_lua->FuncRegisterOptional("gentlySetPanelBrightness", gentlySetPanelBrightness, 0, 4);
  m_lua->FuncRegister("setPanelManaged", setManaged);
  m_lua->FuncRegister("isPanelManaged", isManaged);
  m_lua->FuncRegister("getCurrentAnimationStorage", getCurrentAnimationStorage);
  m_lua->FuncRegister("getPanelCurrentFace", getCurrentFace);
  m_lua->FuncRegister("drawPanelCurrentFrame", DrawCurrentFrame);
  m_lua->FuncRegister("setPanelBrightness", setPanelBrightness);
  m_lua->FuncRegister("getPanelBrightness", getPanelBrightness);
  m_lua->FuncRegister("setInterruptFrames", setInterruptFrames);  
  m_lua->FuncRegister("setInterruptAnimationPin", setInterruptAnimationPin); 
  m_lua->FuncRegisterFromObjectOpt("setRainbowShader", &g_animation, &Animation::setRainbowShader, true); 
  m_lua->FuncRegisterFromObjectOpt("getAnimationStackSize", &g_animation, &Animation::getAnimationStackSize); 
  m_lua->FuncRegister("color565", color565);
  m_lua->FuncRegister("color444", color444);
  m_lua->FuncRegister("getFrameOffsetByName", GetOffsetByName);
  m_lua->FuncRegister("getFrameCountByName", GetFrameCountByName);
  #endif
  m_lua->FuncRegister("decodePng", decodePng); 
  //Aarduino
  m_lua->FuncRegister("tone", Devices::BuzzerTone);
  m_lua->FuncRegister("toneDuration", Devices::BuzzerToneDuration);
  m_lua->FuncRegister("noTone", Devices::BuzzerNoTone);
  m_lua->FuncRegister("i2cScan", Devices::I2CScan);
  m_lua->FuncRegister("millis", millis);
  m_lua->FuncRegister("delay", delay);
  m_lua->FuncRegister("digitalWrite", digitalWrite);
  m_lua->FuncRegister("digitalRead", digitalRead);
  m_lua->FuncRegister("vTaskDelay", vTaskDelay);
  m_lua->FuncRegister("analogRead", analogRead);
  m_lua->FuncRegister("pinMode", pinMode);
  m_lua->FuncRegister("restart", restart);
  m_lua->FuncRegister("delayMicroseconds", delayMicroseconds);
  m_lua->FuncRegister("setBrownoutDetection", setBrownoutDetection);
  m_lua->FuncRegister("getResetReason", getResetReason);
  m_lua->FuncRegister("restart", restart);
  //Serial
  m_lua->FuncRegisterOptional("beginSerialIo", beginSerialIo, 115200);
  m_lua->FuncRegister("setTimeoutSerialIo", setTimeoutSerialIo);
  m_lua->FuncRegister("serialIoAvaliable", serialIoAvaliable);
  m_lua->FuncRegister("serialAvaliable", serialAvaliable);
  m_lua->FuncRegisterOptional("serialReadStringUntil", serialReadStringUntil, '\n');
  m_lua->FuncRegisterOptional("serialIoReadStringUntil", serialIoReadStringUntil, '\n');
  m_lua->FuncRegister("serialIoRead", serialIoRead);
  m_lua->FuncRegister("serialRead", serialRead);
  m_lua->FuncRegister("serialIoWrite", serialIoWrite);
  m_lua->FuncRegister("serialWrite", serialWrite);
  m_lua->FuncRegister("serialIoAvailableForWrite", serialIoAvailableForWrite);
  m_lua->FuncRegister("serialAvailableForWrite", serialAvailableForWrite);
  m_lua->FuncRegister("serialIoWriteString", serialIoWriteString);
  m_lua->FuncRegister("serialWriteString", serialWriteString);
  //I2C
  m_lua->FuncRegister("wireAvailable", wireAvailable);
  m_lua->FuncRegister("wireBegin", wireBegin);
  m_lua->FuncRegister("wireFlush", wireFlush);
  m_lua->FuncRegister("wireBeginTransmission", wireBeginTransmission);
  m_lua->FuncRegisterOptional("wireEndTransmission", wireEndTransmission, true);
  m_lua->FuncRegister("wireRead", wireRead);
  m_lua->FuncRegister("wireReadBytes", wireReadBytes);
  m_lua->FuncRegisterOptional("wireRequestFrom", wireRequestFrom, true);
  m_lua->FuncRegister("wirePeek", wirePeek);
  m_lua->FuncRegister("wireParseFloat", wireParseFloat);
  m_lua->FuncRegister("wireParseInt", wireParseInt);
  m_lua->FuncRegister("wireSetTimeout", wireSetTimeout);
  m_lua->FuncRegister("wireGetTimeout", wireGetTimeout);
  //debug
  m_lua->FuncRegisterRaw("dumpStackToSerial", dumpStackToSerial);
  //Leds
  m_lua->FuncRegisterFromObjectOpt("ledsGetBrightness", &g_leds, &LedStrip::getBrightness);
  m_lua->FuncRegisterFromObjectOpt("ledsSetBrightness", &g_leds, &LedStrip::setBrightness, (uint8_t)128);
  m_lua->FuncRegisterFromObjectOpt("ledsBegin", &g_leds, &LedStrip::Begin, (uint8_t)128);
  m_lua->FuncRegisterFromObjectOpt("ledsBeginDual", &g_leds, &LedStrip::BeginDual, (uint8_t)128);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentRange", &g_leds, &LedStrip::setSegmentRange, 0);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentBehavior", &g_leds, &LedStrip::setSegmentBehavior, 0, 0, 0, 0);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentTweenBehavior", &g_leds, &LedStrip::setSegmentTweenBehavior, 0, 0, 0, 0);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentTweenSpeed", &g_leds, &LedStrip::setSegmentTweenSpeed);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentColor", &g_leds, &LedStrip::setSegmentColor);
  m_lua->FuncRegisterFromObjectOpt("ledsSetColor", &g_leds, &LedStrip::setLedColor);
  m_lua->FuncRegisterFromObjectOpt("ledsDisplay", &g_leds, &LedStrip::Display);
  m_lua->FuncRegisterFromObjectOpt("ledsSetManaged", &g_leds, &LedStrip::SetManaged);
  m_lua->FuncRegisterFromObjectOpt("ledsIsManaged", &g_leds, &LedStrip::IsManaged);
  m_lua->FuncRegisterFromObjectOpt("ledsGentlySeBrightness", &g_leds, &LedStrip::GentlySeBrightness, 0, 2);
  m_lua->FuncRegisterFromObjectOpt("ledsStackCurrentBehavior", &g_leds, &LedStrip::StackBehavior);
  m_lua->FuncRegisterFromObjectOpt("ledsPopBehavior", &g_leds, &LedStrip::PopBehavior);
  //Files
  m_lua->FuncRegister("listFiles", listFiles);
  m_lua->FuncRegister("moveFile", moveFile);
  m_lua->FuncRegister("removeFile", removeFile);
  m_lua->FuncRegister("createDir", createDir);
  m_lua->FuncRegister("fileExists", fileExists);
  
  m_lua->FuncRegister("formatFFAT", formatFFAT);
  #ifdef ENABLE_HUB75_PANEL
  m_lua->FuncRegister("composeBulkFile", composeBulkFile);
  m_lua->FuncRegister("deleteBulkFile", deleteBulkFile);
  #endif
}

void LuaInterface::RegisterConstants()
{

  m_lua->setConstant("BEHAVIOR_NONE", (int)BEHAVIOR_NONE);
  m_lua->setConstant("BEHAVIOR_PRIDE", (int)BEHAVIOR_PRIDE);
  m_lua->setConstant("BEHAVIOR_ROTATE", (int)BEHAVIOR_ROTATE);
  m_lua->setConstant("BEHAVIOR_RANDOM_COLOR", (int)BEHAVIOR_RANDOM_COLOR);
  m_lua->setConstant("BEHAVIOR_FADE_CYCLE", (int)BEHAVIOR_FADE_CYCLE);
  m_lua->setConstant("BEHAVIOR_ROTATE_FADE_CYCLE", (int)BEHAVIOR_ROTATE_FADE_CYCLE);
  m_lua->setConstant("BEHAVIOR_STATIC_RGB", (int)BEHAVIOR_STATIC_RGB);
  m_lua->setConstant("BEHAVIOR_STATIC_HSV", (int)BEHAVIOR_STATIC_HSV);
  m_lua->setConstant("BEHAVIOR_RANDOM_BLINK", (int)BEHAVIOR_RANDOM_BLINK);
  m_lua->setConstant("BEHAVIOR_ICON_X", (int)BEHAVIOR_ICON_X);
  m_lua->setConstant("BEHAVIOR_ICON_I", (int)BEHAVIOR_ICON_I);
  m_lua->setConstant("BEHAVIOR_ICON_V", (int)BEHAVIOR_ICON_V);
  m_lua->setConstant("BEHAVIOR_ROTATE_SINE_V", (int)BEHAVIOR_ROTATE_SINE_V);
  m_lua->setConstant("BEHAVIOR_ROTATE_SINE_S", (int)BEHAVIOR_ROTATE_SINE_S);
  m_lua->setConstant("BEHAVIOR_ROTATE_SINE_H", (int)BEHAVIOR_ROTATE_SINE_H);
  m_lua->setConstant("BEHAVIOR_FADE_IN", (int)BEHAVIOR_FADE_IN);
  m_lua->setConstant("BEHAVIOR_NOISE", (int)BEHAVIOR_NOISE);
  m_lua->setConstant("MAX_LED_GROUPS", (int)MAX_LED_GROUPS);


  m_lua->setConstant("POWER_MODE_USB_5V", (int)POWER_MODE_USB_5V);
  m_lua->setConstant("POWER_MODE_USB_9V", (int)POWER_MODE_USB_9V);
  m_lua->setConstant("POWER_MODE_BATTERY", (int)POWER_MODE_BATTERY);
  m_lua->setConstant("POWER_MODE_REGULATOR_PD", (int)POWER_MODE_REGULATOR_PD);

  m_lua->setConstant("BLACK", (int)BLACK);
  m_lua->setConstant("WHITE", (int)WHITE);

  m_lua->setConstant("D1", 1);
  m_lua->setConstant("D2", 2);

  m_lua->setConstant("HIGH", 1);
  m_lua->setConstant("LOW", 0);
  m_lua->setConstant("INPUT", INPUT); 
  m_lua->setConstant("OUTPUT", OUTPUT);
  m_lua->setConstant("INPUT_PULLUP", INPUT_PULLUP);
  m_lua->setConstant("INPUT_PULLDOWN", INPUT_PULLDOWN);

  m_lua->setConstant("ANALOG", ANALOG);
  m_lua->setConstant("OUTPUT_OPEN_DRAIN", OUTPUT_OPEN_DRAIN);
  m_lua->setConstant("OPEN_DRAIN", OPEN_DRAIN);
  m_lua->setConstant("PULLDOWN", PULLDOWN);

  m_lua->setConstant("ESP_RST_UNKNOWN", (int)ESP_RST_UNKNOWN);
  m_lua->setConstant("ESP_RST_POWERON", (int)ESP_RST_POWERON);
  m_lua->setConstant("ESP_RST_EXT", (int)ESP_RST_EXT);
  m_lua->setConstant("ESP_RST_SW", (int)ESP_RST_SW);
  m_lua->setConstant("ESP_RST_PANIC", (int)ESP_RST_PANIC);
  m_lua->setConstant("ESP_RST_INT_WDT", (int)ESP_RST_INT_WDT);
  m_lua->setConstant("ESP_RST_TASK_WDT", (int)ESP_RST_TASK_WDT);
  m_lua->setConstant("ESP_RST_WDT", (int)ESP_RST_WDT);
  m_lua->setConstant("ESP_RST_DEEPSLEEP", (int)ESP_RST_DEEPSLEEP);
  m_lua->setConstant("ESP_RST_BROWNOUT", (int)ESP_RST_BROWNOUT);
  m_lua->setConstant("ESP_RST_SDIO", (int)ESP_RST_SDIO);

  m_lua->setConstant("PANDA_VERSION", PANDA_VERSION);
  m_lua->setConstant("BUILT_IN_POWER_MODE", (int)BUILT_IN_POWER_MODE);
  #ifdef PIN_ENABLE_REGULATOR
  m_lua->setConstant("PIN_ENABLE_REGULATOR", (int)PIN_ENABLE_REGULATOR);
  #else
  m_lua->setConstant("PIN_ENABLE_REGULATOR", -1);
  #endif
  m_lua->setConstant("PIN_USB_BATTERY_IN", (int)PIN_USB_BATTERY_IN);
  m_lua->setConstant("RESISTOR_DIVIDER_R8", (float)RESISTOR_DIVIDER_R8);
  m_lua->setConstant("RESISTOR_DIVIDER_R9", (float)RESISTOR_DIVIDER_R9);
  m_lua->setConstant("V_REF", (float)V_REF);
  m_lua->setConstant("VCC_THRESHOLD_START", VCC_THRESHOLD_START);
  m_lua->setConstant("VCC_THRESHOLD_HALT", VCC_THRESHOLD_HALT);
  m_lua->setConstant("OLED_SCREEN_WIDTH", OLED_SCREEN_WIDTH);
  m_lua->setConstant("OLED_SCREEN_HEIGHT", OLED_SCREEN_HEIGHT);
  m_lua->setConstant("PANEL_WIDTH", PANEL_WIDTH);
  m_lua->setConstant("PANEL_HEIGHT", PANEL_HEIGHT);
  m_lua->setConstant("MAX_BLE_BUTTONS", (int)MAX_BLE_BUTTONS);
  m_lua->setConstant("MAX_BLE_CLIENTS", (int)MAX_BLE_CLIENTS);
  m_lua->setConstant("SERVO_COUNT", (int)SERVO_COUNT);
  m_lua->setConstant("MAX_LED_GROUPS", MAX_LED_GROUPS);
  m_lua->setConstant("EDIT_MODE_PIN", EDIT_MODE_PIN);
  m_lua->setConstant("WIFI_AP_NAME", WIFI_AP_NAME);
  m_lua->setConstant("WIFI_AP_PASSWORD", WIFI_AP_PASSWORD);
  m_lua->setConstant("EDIT_MODE_FTP_USER", EDIT_MODE_FTP_USER);
  m_lua->setConstant("EDIT_MODE_FTP_PASSWORD", EDIT_MODE_FTP_PASSWORD);
  m_lua->setConstant("EDIT_MODE_FTP_PORT", EDIT_MODE_FTP_PORT);
  m_lua->setConstant("SERVO_COUNT", SERVO_COUNT);
  m_lua->setConstant("PANEL_CHAIN", PANEL_CHAIN);

  #ifdef ENABLE_HUB75_PANEL
  m_lua->setConstant("COLOR_MODE_RGB", (int)COLOR_MODE_RGB);
  m_lua->setConstant("COLOR_MODE_RBG", (int)COLOR_MODE_RBG);
  m_lua->setConstant("COLOR_MODE_GRB", (int)COLOR_MODE_GRB);
  m_lua->setConstant("COLOR_MODE_GBR", (int)COLOR_MODE_GBR);
  m_lua->setConstant("COLOR_MODE_BRG", (int)COLOR_MODE_BRG);
  m_lua->setConstant("COLOR_MODE_BGR", (int)COLOR_MODE_BGR);
  #endif

}

bool LuaInterface::Start()
{

  m_lua = new (LuaWrapper);
  if (!m_lua)
  {
    return false;
  }

  m_lua->SetErrorCallback(LuaInterface::luaCallbackError);

  RegisterMethods();
  RegisterConstants();

  auto _state = m_lua->GetState();

  
  
  ClassRegister<Batata>::RegisterClassType(_state,"Batata",[](lua_State* L){
        Batata *t = new Batata();
        return t;
  });
    
  ClassRegister<Batata>::RegisterClassMethod(_state,"Batata","Get",&Batata::Get);
  ClassRegister<Batata>::RegisterClassMethod(_state,"Batata","Sum",&Batata::Sum, 0);
  ClassRegister<Batata>::RegisterClassMethod(_state,"Batata","Set",&Batata::Set);
  ClassRegister<Batata>::RegisterClassMethod(_state,"Batata","SumBatata",&Batata::SumBatata);
  ClassRegister<Batata>::RegisterClassMethod(_state,"Batata","CloneBatata",&Batata::CloneBatata);

  ClassRegister<Batata>::RegisterField(_state, "count", &Batata::count);



  static LuaCFunctionLambda EmptyGC = [](lua_State* L) -> int{
    return 0;
  };
  
  ClassRegister<BleServiceHandler>::RegisterClassType(_state,"BleServiceHandler",[](lua_State* L) -> BleServiceHandler*{
    if (lua_gettop(L) == 2){ 
      bool hasErr;
      std::string service = GenericLuaGetter<std::string>::Call(hasErr, L);
      if (service.length() != 36 ) {
        luaL_error(L, "Malformed service UUID");
        return nullptr;
      }
      NimBLEUUID uuid(service);

      if (uuid == NimBLEUUID()){
        luaL_error(L, "Malformed service UUID");
        return nullptr;
    } 

      BleServiceHandler *obj = new BleServiceHandler(uuid.to16());
      g_remoteControls.AddAcceptedService(uuid.to16().toString().c_str(), obj);
      Logger::Info("Accepting service: %s", uuid.to16().toString().c_str());
      return obj;
    }else{
      luaL_error(L, "Missing UUID parameter");
      return nullptr;
    }
  }, &EmptyGC);
  ClassRegister<BleServiceHandler>::RegisterClassMethod(_state,"BleServiceHandler","AddCharacteristics",&BleServiceHandler::AddCharacteristics);
  ClassRegister<BleServiceHandler>::RegisterClassMethod(_state,"BleServiceHandler","SetOnConnectCallback",&BleServiceHandler::SetOnConnectCallback);
  ClassRegister<BleServiceHandler>::RegisterClassMethod(_state,"BleServiceHandler","WriteToCharacteristics",&BleServiceHandler::WriteToCharacteristics, true);


  ClassRegister<BleCharacteristicsHandler>::RegisterClassType(_state,"BleCharacteristicsHandler",[](lua_State* L){ luaL_error(L, "Cannot create a empty object of this class"); return nullptr;}, &EmptyGC);
  ClassRegister<BleCharacteristicsHandler>::RegisterClassMethod(_state,"BleCharacteristicsHandler","SetSubscribeCallback",&BleCharacteristicsHandler::SetSubscribeCallback);
  ClassRegister<BleCharacteristicsHandler>::RegisterClassMethod(_state,"BleCharacteristicsHandler","SetCallbackModeStream",&BleCharacteristicsHandler::SetCallbackModeStream);
  
  lastError = "";

  return true;
}

String LuaInterface::getLastReturnAsString(){
  lua_State* L = m_lua->GetState();
  int top = lua_gettop(L);
  
  if (top == 0) {
    return "";
  }

  String response = "";
  for (int i = 1; i <= top; i++) {
    if (i > 1) {
      response += "\t";
    }

    int type = lua_type(L, i);
    switch (type) {
      case LUA_TNIL:
        response += "nil";
        break;
      case LUA_TBOOLEAN:
        response += lua_toboolean(L, i) ? "true" : "false";
        break;
      case LUA_TNUMBER:
        response += String(lua_tonumber(L, i));
        break;
      case LUA_TSTRING:
        response += String(lua_tostring(L, i));
        break;
      case LUA_TTABLE:
        response += "[table]";
        break;
      case LUA_TFUNCTION:
        response += "[function]";
        break;
      case LUA_TUSERDATA:
        response += "[userdata]";
        break;
      case LUA_TTHREAD:
        response += "[thread]";
        break;
      case LUA_TLIGHTUSERDATA:
        response += "[lightuserdata]";
        break;
      default:
        response += "[unknown]";
        break;
    }
  }
  lua_settop(L, 0);
  return response;
}

bool LuaInterface::DoString(const char *content, int returns)
{
  return m_lua->Lua_dostring(content, returns);
}

bool LuaInterface::LoadFile(const char *functionName)
{
  if (luaL_dofile(m_lua->GetState(), functionName))
  {
    const char *error_message = lua_tostring(m_lua->GetState(), -1);
    Serial.printf("Error loading file %s:\n%s", functionName, error_message);
    Logger::Info("Lua error: %s\n", error_message);
    return false;
  }
  return true;
}

bool LuaInterface::CallFunction(const char *functionName)
{
  return m_lua->callLuaFunction(functionName);
}