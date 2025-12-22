#include "drawing/dma_display.hpp"

#ifdef ENABLE_HUB75_PANEL

MatrixPanel_I2S_DMA_2 *DMADisplay::Display = nullptr;

HUB75_I2S_CFG DMADisplay::mxconfig(
    PANEL_WIDTH,   // module width
    PANEL_HEIGHT,   // module height
    PANEL_CHAIN    // Chain length
  );


bool DMADisplay::Start(uint8_t colordepth, uint8_t chainLenght){
  if (DMADisplay::Display != nullptr){
    return false;
  }
  mxconfig.chain_length = chainLenght;
  mxconfig.double_buff = true; // Turn of double buffer
  mxconfig.clkphase = false;

  mxconfig.gpio.r1 = DMA_GPIO_R1;
  mxconfig.gpio.g1 = DMA_GPIO_G1;
  mxconfig.gpio.b1 = DMA_GPIO_B1;
  mxconfig.gpio.r2 = DMA_GPIO_R2;
  mxconfig.gpio.g2 = DMA_GPIO_G2;
  mxconfig.gpio.b2 = DMA_GPIO_B2;
  mxconfig.gpio.a = DMA_GPIO_A;
  mxconfig.gpio.b = DMA_GPIO_B;
  mxconfig.gpio.c = DMA_GPIO_C;
  mxconfig.gpio.d = DMA_GPIO_D;
  mxconfig.gpio.lat = DMA_GPIO_LAT;
  mxconfig.gpio.oe = DMA_GPIO_OE;
  mxconfig.gpio.clk = DMA_GPIO_CLK;

  mxconfig.setPixelColorDepthBits(colordepth);
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_20M;
  // Display Setup
  DMADisplay::Display = new MatrixPanel_I2S_DMA_2(mxconfig);
  if (DMADisplay::Display == nullptr){
    return false;
  }

  DMADisplay::Display->begin();
  DMADisplay::Display->setBrightness8(1);
  DMADisplay::Display->clearScreen();
  DMADisplay::Display->flipDMABuffer();
  DMADisplay::Display->clearScreen();
  DMADisplay::Display->flipDMABuffer();
  return true;
}


void DMADisplay::DrawTestScreen(){
    DMADisplay::Display->clearScreen();
    for (int i=0;i<16;i++){
      DMADisplay::Display->fillRect(4 * i, 0, 4, 9, DMADisplay::Display->color444(i, 0, 0));
      DMADisplay::Display->fillRect(4 * i, 11, 4, 10, DMADisplay::Display->color444(0, i, 0));
      DMADisplay::Display->fillRect(4 * i, 8, 4, 3, DMADisplay::Display->color444(i, i, 0));
      DMADisplay::Display->fillRect(4 * i, 21, 4, 7, DMADisplay::Display->color444(0, 0, i));
      DMADisplay::Display->fillRect(4 * i, 18, 4, 3, DMADisplay::Display->color444(0, i, i));
      DMADisplay::Display->fillRect(4 * i, 28, 4, 2, DMADisplay::Display->color444(i, i, i));

      DMADisplay::Display->fillRect(127-4 * i, 0, 4, 9, DMADisplay::Display->color444(i, 0, 0));
      DMADisplay::Display->fillRect(127-4 * i, 11, 4, 10, DMADisplay::Display->color444(0, i, 0));
      DMADisplay::Display->fillRect(127-4 * i, 8, 4, 3, DMADisplay::Display->color444(i, i, 0));
      DMADisplay::Display->fillRect(127-4 * i, 21, 4, 7, DMADisplay::Display->color444(0, 0, i));
      DMADisplay::Display->fillRect(127-4 * i, 18, 4, 3, DMADisplay::Display->color444(0, i, i));
      DMADisplay::Display->fillRect(127-4 * i, 28, 4, 2, DMADisplay::Display->color444(i, i, i));
    }
    DMADisplay::Display->flipDMABuffer();
}

#endif