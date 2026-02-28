#include "tools/hardwareconfig.hpp"
#include "tools/oledscreen.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include "tools/psrammap.hpp"

HUB75_I2S_CFG HardwareConfig::panelConfig(
    PANEL_WIDTH,  
    PANEL_HEIGHT,   
    PANEL_CHAIN 

);

void HardwareConfig::loadDefaults(){
    panelConfig.double_buff = true; // Turn of double buffer
    panelConfig.clkphase = false;
    panelConfig.gpio.r1 = DMA_GPIO_R1;
    panelConfig.gpio.g1 = DMA_GPIO_G1;
    panelConfig.gpio.b1 = DMA_GPIO_B1;
    panelConfig.gpio.r2 = DMA_GPIO_R2;
    panelConfig.gpio.g2 = DMA_GPIO_G2;
    panelConfig.gpio.b2 = DMA_GPIO_B2;
    panelConfig.gpio.a = DMA_GPIO_A;
    panelConfig.gpio.b = DMA_GPIO_B;
    panelConfig.gpio.c = DMA_GPIO_C;
    panelConfig.gpio.d = DMA_GPIO_D;
    panelConfig.gpio.lat = DMA_GPIO_LAT;
    panelConfig.gpio.oe = DMA_GPIO_OE;
    panelConfig.gpio.clk = DMA_GPIO_CLK;
    panelConfig.setPixelColorDepthBits(12);
    panelConfig.i2sspeed = HUB75_I2S_CFG::HZ_20M;
}


void HardwareConfig::loadServosAndStart(JsonObject servos){
    if (!servos["enabled"]) {
        return;
    }
    
    if (!servos["pins"].is<JsonArray>()) {
        return;
    }
    
    JsonArray pinsArray = servos["pins"].as<JsonArray>();
    
    if (pinsArray.size() == 0) {
        return;
    }
    
    std::vector<int> pins;
    for (JsonVariant pin : pinsArray) {
        if (pin.is<int>()) {
            pins.push_back(pin.as<int>());
        }
    }
    
    if (pins.size() > 0) {
        Devices::StartServos(pins);
    }
}

void HardwareConfig::loadHub75AndStart(JsonObject hub75){
    if (!hub75["enabled"]) {
        return;
    }

    if (hub75.containsKey("dma_r1")) panelConfig.gpio.r1 = hub75["dma_r1"];
    if (hub75.containsKey("dma_g1")) panelConfig.gpio.g1 = hub75["dma_g1"];
    if (hub75.containsKey("dma_b1")) panelConfig.gpio.b1 = hub75["dma_b1"];
    if (hub75.containsKey("dma_r2")) panelConfig.gpio.r2 = hub75["dma_r2"];
    if (hub75.containsKey("dma_g2")) panelConfig.gpio.g2 = hub75["dma_g2"];
    if (hub75.containsKey("dma_b2")) panelConfig.gpio.b2 = hub75["dma_b2"];
    if (hub75.containsKey("dma_a")) panelConfig.gpio.a = hub75["dma_a"];
    if (hub75.containsKey("dma_b")) panelConfig.gpio.b = hub75["dma_b"];
    if (hub75.containsKey("dma_c")) panelConfig.gpio.c = hub75["dma_c"];
    if (hub75.containsKey("dma_d")) panelConfig.gpio.d = hub75["dma_d"];
    if (hub75.containsKey("dma_lat")) panelConfig.gpio.lat = hub75["dma_lat"];
    if (hub75.containsKey("dma_oe")) panelConfig.gpio.oe = hub75["dma_oe"];
    if (hub75.containsKey("dma_clk")) panelConfig.gpio.clk = hub75["dma_clk"];
        
    if (hub75.containsKey("colordepth")) {
        panelConfig.setPixelColorDepthBits(hub75["colordepth"]);
    }

    StartDmaDisplay();
    return;
}

bool HardwareConfig::LoadConfigs(){
    loadDefaults();

    File conf = SD.open( "/hardware.json" );
    if( !conf ) {
        OledScreen::CriticalFail("Can't open hardware.jsonn");
        for(;;){}
        return false;
    }

    SpiRamAllocator allocator;
    JsonDocument  hardwareConfigJson(&allocator);
    auto err = deserializeJson( hardwareConfigJson, conf );
    conf.close();
    if( err ) {
       char miniHBuffer[1000];
       sprintf(miniHBuffer, "hardware.json.json:\n%s", err.c_str());
       OledScreen::CriticalFail(miniHBuffer);
       for(;;){}
       return false;
    }

    if (hardwareConfigJson.containsKey("hub75")) {
        loadHub75AndStart(hardwareConfigJson["hub75"]);
    }

    if (hardwareConfigJson.containsKey("servos")) {
        loadServosAndStart(hardwareConfigJson["servos"]);
    }

    if (Devices::Display == nullptr){
        Devices::Display = new MockDisplay(panelConfig);
    }



    hardwareConfigJson.clear();

    return true;
}

bool HardwareConfig::StartDmaDisplay(){
    #ifdef ENABLE_HUB75_PANEL
    if (Devices::Display != nullptr){
        Logger::Info("DMA display is already started.");
        return false;
    }
    //Todo check avaliable ram
    Devices::Display = new MatrixPanel_I2S_DMA_2(panelConfig);
    if (Devices::Display == nullptr){
        Logger::Info("Failed to start DMA display");
        return false;
    }

    Devices::Display->begin();
    Devices::Display->setBrightness8(1);
    Devices::Display->clearScreen();
    Devices::Display->flipDMABuffer();
    Devices::Display->clearScreen();
    Devices::Display->flipDMABuffer();
    Logger::Info("DMA display initialized!");
    Devices::CalculateMemmoryUsageDifference("Dma display");

    return true;
    #else 
    return false;
    #endif
}
