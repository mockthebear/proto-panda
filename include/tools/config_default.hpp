#pragma once
/*
    Avoid changing this vile, change the config.hpp instead if you need custom configuration
*/
#define PANDA_VERSION "3.3.7"
/*
Cache file version to invalidate cache in case of firmware update
*/
#define PANDA_CACHE_VERSION 15
/*
    Pin to enable the buck converter.
    With this pin on HIGH the buck converter will start regulating the USB/Battery input
    to the desired 5v out for the panels.
*/
//#define USE_ENABLE_PIN 
#define PIN_ENABLE_REGULATOR 13
#define BUILT_IN_POWER_MODE POWER_MODE_NONE
/*
    Run all tasks on a single core. This can help leave the BLE to have a dedicated core and this will avoid crashes related to BLE on crowded areas.
*/
//#define SINGLE_CORE_RUN 1
/*
    This pin is the pin where the input will be read from the voltage divider.
    The resistors are R9 and R8 (3k and 10k)

*/
//#define USE_PIN_BATTERY_IN
#define PIN_USB_BATTERY_IN 3
/*
    R8 is 10k
*/
#define RESISTOR_DIVIDER_R8 10000.00
/*
    R9 is 3k
*/
#define RESISTOR_DIVIDER_R9 3000.0  
/*
    ESP32 adc vref is 3.3v
*/
#define V_REF 3.3
/*
    Minimum voltage to start working
*/
#define VCC_THRESHOLD_START 7.5f
/*
    Minimum voltage to stop working
*/
#define VCC_THRESHOLD_HALT 6.0f

/*
    I2C
*/

#define I2C_SDA 8
#define I2C_SLC 9

/*
    SD CARD MODE
*/
//If you're using a SD card module, change this to 1.
//But if you're using a smd assembled version, you can leave it as 2
#define PANDA_SD_MODE 2

/*
    SPI
*/

#define SPI_CS 38
#define SPI_MOSI 14
#define SPI_MISO 47
#define SPI_SCK 21
#define SPI_MAX_CLOCK (40 * 1000 * 1000)

/* 
    SD_MMC
*/
#define MMC_PIN_DATA2 -1
#define MMC_PIN_DATA3 SPI_CS
#define MMC_PIN_CMD SPI_MOSI
#define MMC_PIN_CLK SPI_SCK
#define MMC_PIN_DATA0 SPI_MISO
#define MMC_PIN_DATA1 -1
#define MMC_CLOCK_SPEED 40000
//If using the full range of pins (data1 and 2, set this to false)
#define MMC_ONE_BIT true  



/*
 Oled screen
*/

#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64 
#define OLED_SCREEN_ADDRESS 0x3C 
#define OLED_SCREEN_ROTATION 2
#define OLED_SCREEN_CLOCK_FREQ 800000


#define USE_LIDAR
#define LIDAR_ADDR 0x29


/*
    Led strip
*/

#define LED_STRIP_PIN_1 41
#define LED_STRIP_PIN_2 42
#define LED_STRIP_TYPE WS2812B
#define MAX_LED_GROUPS 16

#define IF_USING_WS2812B_MATRIX_SCREEN_PIN 41

/*
    Buzzer
*/
#define USE_BUZZER 1
#define BUZZER_PIN 40
#define BUZZER_CHANNEL 7
/*
    Edit mode pin
*/
#define ENABLE_EDIT_MODE
#define EDIT_MODE_PIN 39
#define USE_BOOT_PIN_FOR_EDIT_MODE 
#define EDIT_ENABLE_LOGIC_LEVEL LOW
/* 
Servos
*/

#define USE_SERVO
#define SERVO_RESOLUTION_BITS 14


//#define USE_INTERNAL_ACCELEROMETER
#define INTERNAL_ACCELEROMETER_ADDR 107

/*
    DMA display, or actual display
*/
#define DEFAULT_CANVAS_WIDTH 64
#define DEFAULT_CANVAS_HEIGHT 32


#define CANVAS_WIDTH (HardwareConfig::CanvasWidth())      // Number of pixels wide of each INDIVIDUAL sprite
#define CANVAS_HEIGHT (HardwareConfig::CanvasHeight())     // Number of pixels tall of each INDIVIDUAL sprite

#define FILE_PIXEL_COUNT (CANVAS_WIDTH * CANVAS_HEIGHT)
#define FILE_SIZE_BULK_SIZE ( FILE_PIXEL_COUNT * sizeof(uint16_t) )
#define FILE_HEADER_BYTES 8
#define FILE_HEADER_SIZE (sizeof(uint8_t) * FILE_HEADER_BYTES )
//File + header size
#define FILE_SIZE (( FILE_SIZE_BULK_SIZE  +  FILE_HEADER_SIZE )) 

#define DMA_GPIO_R1 12

#define DMA_GPIO_G1 10
#define DMA_GPIO_B1 11

#define DMA_GPIO_R2 20

#define DMA_GPIO_G2 18
#define DMA_GPIO_B2 19

#define DMA_GPIO_A 17
#define DMA_GPIO_B 16
#define DMA_GPIO_C 15
#define DMA_GPIO_D 7
#define DMA_GPIO_LAT 5
#define DMA_GPIO_OE 4
#define DMA_GPIO_CLK 6


#define MAX7219_SIZE 8

#if PANDA_SD_MODE == 1
#define PANDA_SD SD
#define PANDA_SD_NAME "SD"
#elif PANDA_SD_MODE == 2
#define PANDA_SD SD_MMC
#define PANDA_SD_NAME "MMC"
#endif

#define ENABLE_LUA
#define ENABLE_BLE