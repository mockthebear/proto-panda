#include "tools/oledscreen.hpp"
#include "tools/logger.hpp"
#include "tools/sensors.hpp"
#include "tools/devices.hpp"
#include "bluetooth/ble_client.hpp"
#include "drawing/animation.hpp"

#include "drawing/icons/icons.hpp"


extern Animation g_animation;
Adafruit_SSD1306 OledScreen::display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, -1);
bool OledScreen::consoleMode = false;
std::list<std::string> OledScreen::lines;
uint8_t *OledScreen::DisplayFace = nullptr;

uint32_t OledScreen::swapTimer = 0;

std::vector<OledIcon> OledScreen::icons;

bool OledScreen::Start(){
    
    OledScreen::DisplayFace = (uint8_t*)ps_malloc(sizeof(uint8_t) * PANEL_WIDTH * PANEL_HEIGHT);

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