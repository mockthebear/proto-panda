#include "tools/oledscreen.hpp"
#include "tools/logger.hpp"
#include "tools/sensors.hpp"
#include "tools/devices.hpp"
#include "bluetooth/ble_client.hpp"
#include "drawing/animation.hpp"

#include "drawing/icons/icons.hpp"

static const unsigned char PROGMEM logo_handy[] =
{ 0xff, 0xf0, 0x80, 0x10, 0x80, 0x10, 0x8a, 0x90, 0x8a, 0x90, 0xaf, 0xd0, 0xbf, 0xd0, 0x9f, 0xd0, 
	0x9f, 0x90, 0x8f, 0x90, 0x8f, 0x90, 0xff, 0xf0
};

static const unsigned char PROGMEM logo_no_handy[] =
{ 0xff, 0xf0, 0xc0, 0x30, 0xa0, 0x50, 0x90, 0x90, 0x89, 0x10, 0x86, 0x10, 0x86, 0x10, 0x89, 0x10, 
	0x90, 0x90, 0xa0, 0x50, 0xc0, 0x30, 0xff, 0xf0
};

static const unsigned char PROGMEM logo_battery[] =
{ 0x00, 0x00, 0x0f, 0x00, 0x3f, 0xc0, 0x20, 0x40, 0x20, 0x40, 0x3f, 0xc0, 0x20, 0x40, 0x20, 0x40, 
0x3f, 0xc0, 0x20, 0x40, 0x20, 0x40, 0x3f, 0xc0
};

extern Animation g_animation;
Adafruit_SSD1306 OledScreen::display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, -1);
bool OledScreen::consoleMode = false;
std::list<std::string> OledScreen::lines;
uint8_t OledScreen::DisplayFace[PANEL_WIDTH * PANEL_HEIGHT];

InfoTypeShown OledScreen::infoShown = SHOW_FPS;
uint32_t OledScreen::swapTimer = 0;

std::vector<OledIcon> OledScreen::icons;

bool OledScreen::Start(){
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
        Serial.println(F("SSD1306 allocation failed"));
        return false;
    }

    display.setRotation(OLED_SCREEN_ROTATION);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.clearDisplay();
    display.printf(":)\n");
    display.display();
    SetConsoleMode(false);


    display.clearDisplay();
    display.drawBitmap(0,0, icon_protopanda, 128, 64, 1);
    
    
    if (BUILT_IN_POWER_MODE == POWER_MODE_USB_5V){
        display.setCursor((OLED_SCREEN_WIDTH/2), OLED_SCREEN_HEIGHT-8);
        display.printf("v%s", PANDA_VERSION);
    }else{
        display.setCursor((OLED_SCREEN_WIDTH/2)-8, OLED_SCREEN_HEIGHT-8);
        display.printf("v%s +9v", PANDA_VERSION);
    }
    display.display();
    delay(1500);
  
    return true;
}

int OledScreen::CreateIcon(std::vector<uint8_t> iconData, int width, int height){
    size_t size = iconData.size();

    int expectedWidth = width/8;
    if (width != expectedWidth){
        expectedWidth++;
        expectedWidth *= 8;
    }

    if ((size*8) != (expectedWidth*height)){
        Logger::Error("Error creating icon with %dx%d, expected %d bytes but got %d", width, height,  (expectedWidth*height), (size*8));
        return -1;
    }

    uint8_t *buff = (uint8_t*)ps_malloc(size * sizeof(uint8_t));
    std::copy(iconData.begin(), iconData.end(), buff);
    OledScreen::icons.emplace_back(OledIcon(width, height, buff));
    return OledScreen::icons.size()-1;
}

void OledScreen::DrawIcon(int x, int y, int iconId){
    if (iconId < 0 || iconId >= OledScreen::icons.size()){
        Logger::Error("Icon id %d is not loaded", iconId);
        return;
    }

    auto ic = &OledScreen::icons[iconId];

    if (ic == nullptr || ic->icon == nullptr){
        return;
    }
    display.drawBitmap(x,y, ic->icon, ic->width, ic->height, 1);
}

void OledScreen::DrawPanelFaceToScreen(int xx, int yy){
    display.drawRect(xx,yy,PANEL_WIDTH+2,PANEL_HEIGHT+2, 1);
    int ptr = 0;
    for (int y=0;y<PANEL_HEIGHT;y++){
      for (int x=0;x<PANEL_WIDTH;x++){
        display.drawPixel(xx+x+1, yy+y+1, OledScreen::DisplayFace[ptr++]);
      }
    }
}

void OledScreen::Clear(){
    if (OledScreen::consoleMode){
        return;
    }
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
}

void OledScreen::DrawBottomBar(){
    if (OledScreen::consoleMode){
        return;
    }
    const int posY = OLED_SCREEN_HEIGHT-18;
    display.drawRect(0,posY, OLED_SCREEN_WIDTH, 17, 0);
    display.drawFastHLine(0,posY,OLED_SCREEN_WIDTH,1);
    display.drawFastHLine(0,posY+17,OLED_SCREEN_WIDTH,1);

    int startX = 0;
    display.drawFastVLine(startX,posY,posY+17,1);
    DrawAccelerometer(0, startX, posY);
    display.drawFastVLine(startX+16,posY,posY+17,1);
    DrawAccelerometer(1, startX+17, posY);
    display.drawFastVLine(startX+33,posY,posY+17,1);

    display.setCursor(startX+35, posY+4);
    switch (infoShown)
    {
        case SHOW_FPS:
            display.printf("Lua FPS: %d", (int)Devices::getFps());
            break;
        case SHOW_FREE_RAM:
            display.printf("F.Ram: %2.2f%%", (Devices::getFreePsram()));
            break;
        case SHOW_PANEL_FPS:
            display.printf("P.FPS: %d", (int)Devices::getAutoFps());
            break;
        case SHOW_FREE_HEAP:
            display.printf("F.Heap: %d", (int)Devices::getFreeHeap());
            break;
        default:
            break;
    }

    if (swapTimer < millis()){
        swapTimer = millis() + 5 * 1000;
        infoShown = InfoTypeShown(int(infoShown)+1);
        if (infoShown == SHOW_RESET){
            infoShown = SHOW_FIRST;
        }
    }

    if (g_remoteControls.isElementIdConnected(0)){
        display.drawBitmap(OLED_SCREEN_WIDTH-12, posY+2, logo_handy, 12,12, SSD1306_WHITE);
    }else{
        display.drawBitmap(OLED_SCREEN_WIDTH-12, posY+2, logo_no_handy, 12,12, SSD1306_WHITE);
    }
    if (g_remoteControls.isElementIdConnected(1)){
        display.drawBitmap(OLED_SCREEN_WIDTH-24, posY+2, logo_handy, 12,12, SSD1306_WHITE);
    }else{
        display.drawBitmap(OLED_SCREEN_WIDTH-24, posY+2, logo_no_handy, 12,12, SSD1306_WHITE);
    }


    display.setCursor(0,0);
}

void OledScreen::DrawAccelerometer(int id, int posX, int posY){
  int zpos = map(BleManager::remoteData[id].z, -8192, 8192, -8,8);
  zpos = zpos > 8 ? 8 : zpos;
  zpos = zpos < -8 ? -8 : zpos;
  int xpos = map(BleManager::remoteData[id].x,-8192, 8192, -8,8);
  xpos = xpos > 8 ? 8 : xpos;
  xpos = xpos < -8 ? -8 : xpos;
  int ypos = map(BleManager::remoteData[id].y,-8192, 8192, -8,8);
  ypos = ypos > 8 ? 8 : ypos;
  ypos = ypos < -8 ? -8 : ypos;
  display.drawLine(posX+8, posY+8, posX+8+xpos, posY+8, SSD1306_WHITE);
  display.drawLine(posX+8, posY+8, posX+8,  posY+8+ypos, SSD1306_WHITE);
  display.drawLine(posX+8, posY+8, posX+8+zpos,  posY+8+zpos, SSD1306_WHITE);
}

void OledScreen::CriticalFail(const char *str){
    int len = strlen(str);
    Logger::Error(str);
    display.clearDisplay();
    display.setCursor(0, 0);
    if (len > 40){
        display.setTextSize(1);
    }else{
        display.setTextSize(2);
    }
    display.printf(str);
    display.display();
    for (;;){
        
    }
}

void OledScreen::DrawCircularProgress(int val, int max, const char *title) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.printf("%s", title);
    
    float percent = val/(float)max;
    display.setCursor(0, 32);
    display.printf("%2.2f %%", percent * 100.0f);

    display.setCursor(0, 48);
    char unitused[] = "K";
    char unittotal[] = "K";
    if (val > 1024*1024){
        val = val / 1024;
        unitused[0] = 'M';
    }
    if (max > 1024*1024){
        max = max / 1024;
        unittotal[0] = 'M';
    }
    display.printf("Used: %2.2f %sb\nTotal: %2.2f %sb", val/1024.0f, unitused, max/1024.0f, unittotal);
    
    const int radius = 20;
    const int centerX = OLED_SCREEN_WIDTH-(radius*2)-1;
    const int centerY = OLED_SCREEN_HEIGHT/2-4; 
    

    display.drawCircle(centerX, centerY, radius, WHITE);
    if (percent > 0) {
        float angle = 360 * percent;
        
        for (float i = 0.0f; i < angle; i += 0.5f) {
            float rad = (i - 90.0f) * PI / 180.0f; 
            int x = centerX + radius * cos(rad);
            int y = centerY + radius * sin(rad);
            display.drawLine(centerX, centerY, x, y, WHITE);
        }
    }
    display.display();
}

void OledScreen::DrawProgressBar(int val, int max, const char *title){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.printf("%s", title);
    display.setCursor(38, 28);
    float percent = val/(float)max;
    display.printf("%2.2f %%", percent * 100.0f );

    display.drawRect(2,38, OLED_SCREEN_WIDTH-4, 19, 1);
    display.fillRect(2, 38, (OLED_SCREEN_WIDTH-4)*percent, 18, 1);
    display.display();
}

void OledScreen::DrawWaitForPower(float volts){
    display.setCursor(0, 0);
    display.clearDisplay();
    display.setTextSize(2);
    display.printf("No volts!\n%fV\nCheck\npower!!!", volts);
    display.display();
}


void OledScreen::SetConsoleMode(bool enable){
    display.setRotation(OLED_SCREEN_ROTATION);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.clearDisplay();
    lines.clear();
    consoleMode = enable;
    if (enable){
        PrintConsole("     [CONSOLE]");
    }
    display.display();
}

void OledScreen::PrintConsole(const char *str){
    if (!OledScreen::consoleMode){
        return;
    }
    display.setRotation(OLED_SCREEN_ROTATION);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    lines.push_back(str);
    if (lines.size() > 7){
      lines.pop_front();  
    }
    for (auto &it : lines){
        display.println(it.c_str());
    }
    display.display();
}

void OledScreen::PrintError(const char *str){
    display.setRotation(OLED_SCREEN_ROTATION);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("     [ERROR]");
    display.println(str);
    display.display();
}