#pragma once
/*
    Avoid changing this vile, change the config.hpp instead if you need custom configuration
*/
#define PANDA_VERSION "1.1.7"
/*
    Pin to enable the buck converter.
    With this pin on HIGH the buck converter will start regulating the USB/Battery input
    to the desired 5v out for the panels.
*/
#define PIN_ENABLE_REGULATOR 13
#define BUILT_IN_POWER_MODE POWER_MODE_REGULATOR_PD
/*
    Run all tasks on a single core. This can help leave the BLE to have a dedicated core and this will avoid crashes related to BLE on crowded areas.
*/
//#define SINGLE_CORE_RUN 1
/*
    This pin is the pin where the input will be read from the voltage divider.
    The resistors are R9 and R8 (3k and 10k)

*/
#define PIN_USB_BATTERY_IN 3
/*
    R8 is 10k
*/
#define RESISTOR_DIVIDER_R8 11560.373
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
    SPI
*/

#define SPI_CS 38
#define SPI_MOSI 14
#define SPI_MISO 47
#define SPI_SCK 21
#define SPI_MAX_CLOCK (80 * 1000 * 1000)


/*
 Oled screen
*/

#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64 
#define OLED_SCREEN_ADDRESS 0x3C 
#define OLED_SCREEN_ROTATION 2


#define USE_LIDAR
#define LIDAR_ADDR 0x29

/*
    BLE Buttons
*/

#define MAX_BLE_BUTTONS 5
#define MAX_BLE_CLIENTS 4

/*
    Led strip
*/

#define LED_STRIP_PIN_1 41
#define LED_STRIP_PIN_2 42
#define LED_STRIP_TYPE WS2812B
#define MAX_LED_GROUPS 16

/*
    Buzzer
*/
#define USE_BUZZER 1
#define BUZZER_PIN 40
#define BUZZER_CHANNEL 7
/*
    Edit mode pin
*/

#define EDIT_MODE_PIN 39
#define WIFI_AP_NAME "protopanda"
#define WIFI_AP_PASSWORD "pandawah"
#define EDIT_MODE_FTP_USER "wah"
#define EDIT_MODE_FTP_PASSWORD "wah"
#define EDIT_MODE_FTP_PORT 21
#define EDIT_MODE_LUA_PORT 7171
#define EDIT_MODE_HTTP_PORT 80

/* 
Servos
*/

// #define USE_SERVO
#define SERVO_COUNT 4


/*
    DMA display, or actual display
*/
#define ENABLE_HUB75_PANEL 1
#define PANEL_WIDTH 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_HEIGHT 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 2      // Total number of panels chained one to another

#define FILE_SIZE_BULK_SIZE ( PANEL_WIDTH * PANEL_HEIGHT * sizeof(uint16_t) )
#define FILE_SIZE (( FILE_SIZE_BULK_SIZE  + sizeof(uint16_t) ))

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

