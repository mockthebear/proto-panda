#include <Arduino.h>
#include <bluefruit.h>
#include <Wire.h>
#include <LSM6DS3.h>
#include "nrf_gpio.h"

extern const uint32_t g_ADigitalPinMap[];

static uint8_t nrfPin(uint8_t port, uint8_t pin) {
  const uint32_t target = NRF_GPIO_PIN_MAP(port, pin);
  for (uint8_t i = 0; i < PINS_COUNT; i++)
    if (g_ADigitalPinMap[i] == target) return i;
  return 0xFF;   // not exposed by this variant
}
  
#ifdef NICENANO
  #define CUSTOM_PINS 1
  #define NICENANO 1
  #define LED_ACTIVE_LOW 0
  #define DEBUG_SERIAL   1

  static const uint8_t D2 = nrfPin(0, 31);
  static const uint8_t D3 = nrfPin(0, 29);
  static const uint8_t D4 = nrfPin(0, 02);
  static const uint8_t D5 = nrfPin(1, 15);
  static const uint8_t D6 = nrfPin(0, 10);
  static const uint8_t D7 = nrfPin(0, 9);

  static const uint8_t I2C_SCL_PIN = nrfPin(0, 17); 
  static const uint8_t I2C_SDA_PIN = nrfPin(0, 20);

  static const uint8_t PIN_LED_1 = nrfPin(0, 15);   
  static const uint8_t PIN_LED_2 = nrfPin(1, 11); 

  // D0/D1 are TX/RX on the nice!nano, so use D2..D7 for the six buttons
  static const uint8_t button_pins[6] = { D2, D3, D4, D5, D6, D7 };
  #define IMU_ADDR_DEFAULT   0x6a
  #define IMU_ADDR_FALLBACK  0x6b

#elif NRF52832_XXAA
  // Custom nRF52832 
  // Go to ~/.platformio/packages/framework-arduinoadafruitnrf52/variants/feather_nrf52832/variant.h
  // Or C:\Users\<you>\.platformio\packages\framework-arduinoadafruitnrf52/variants/feather_nrf52832/variant.h
  //And comment the line '#define USE_LFXO' and uncomment the '#define USE_LFRC'

  #define LED_ACTIVE_LOW 0
  #define DEBUG_SERIAL   0   

  static const uint8_t PIN_LED_1     = nrfPin(0, 2);
  static const uint8_t PIN_LED_2     = nrfPin(0, 3);
  static const uint8_t I2C_SCL_PIN   = nrfPin(0, 26);
  static const uint8_t I2C_SDA_PIN   = nrfPin(0, 25);

  static const uint8_t button_pins[6] = {
    nrfPin(0, 30), nrfPin(0, 31), nrfPin(0, 7),
    nrfPin(0, 6),  nrfPin(0, 5),  nrfPin(0, 29)
  };
  #define IMU_ADDR_DEFAULT   0x6b
  #define IMU_ADDR_FALLBACK  0x6a
#elif defined(XIAO)
  #define LED_ACTIVE_LOW 1
  #define DEBUG_SERIAL   1
  #define PIN_LED_1 LED_GREEN
  #define PIN_LED_2 LED_BLUE
  static const uint8_t button_pins[6] = { D0, D1, D2, D3, D6, D7 };
  #define IMU_ADDR_DEFAULT   0x6a
  #define IMU_ADDR_FALLBACK  0x6b
#else
 #error No valid board!
#endif

// ---------------- Logging ----------------
#if DEBUG_SERIAL
  #define LOG_BEGIN()   Serial.begin(115200)
  #define LOGF(...)     Serial.printf(__VA_ARGS__)
#else
  #define LOG_BEGIN()   do {} while (0)
  #define LOGF(...)     do {} while (0)
#endif

#define SERVICE_UUID      "d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1"
#define CONFIG_CHAR_UUID  "d4d3fafb-c4c1-c2c3-b4b3-b2b1a4a3a2a1"
#define MEMS_CHAR_UUID    "d4d3afaf-c4c1-c2c3-b4b3-b2b1a4a3a2a1" 

#define DEVICE_NAME   "Proto panda hand 0xc1"
#define STREAM_SIZE   11
#define TICK_MS       50  
#define SLEEP_TICKS   2001 


LSM6DS3* imu = nullptr;
static uint8_t imuAddr = 0;

BLEService        svc(SERVICE_UUID);
BLECharacteristic configChar(CONFIG_CHAR_UUID);
BLECharacteristic memsChar(MEMS_CHAR_UUID);

static bool hasLSM6 = false;
static volatile bool isConnected = false;
static volatile bool received_config_integer = false;
static volatile uint16_t config_integer = 0xffff;

static int32_t advCountToSleep = 0;
static int rebeginCycles = 50;
static int cycleLed = PIN_LED_1;
static bool canCycle = false;

static int16_t AccValue[STREAM_SIZE];

static inline void led(int pin, bool on) {
#if LED_ACTIVE_LOW
  digitalWrite(pin, on ? LOW : HIGH);
#else
  digitalWrite(pin, on ? HIGH : LOW);
#endif
}

static bool tryLSM(uint8_t addr) {
  imu = new LSM6DS3(I2C_MODE, addr);

  imu->settings.gyroEnabled      = 1;
  imu->settings.gyroRange        = 2000;
  imu->settings.gyroSampleRate   = 104;
  imu->settings.gyroBandWidth    = 100;

  imu->settings.accelEnabled     = 1;
  imu->settings.accelRange       = 4;     // note: library uses g value, not the enum
  imu->settings.accelSampleRate  = 104;
  imu->settings.accelBandWidth   = 50;    // 50 Hz roughly matches ODR/2; see below
  
  if (imu->begin() == 0) {
    imuAddr = addr;
    return true;
  }
  LOGF("LSM6 not found at 0x%02X\n", addr);
  delete imu;
  imu = nullptr;
  return false;
}

static bool initLSM() {
  Wire.setClock(400000); 
  if (tryLSM(IMU_ADDR_DEFAULT) || tryLSM(IMU_ADDR_FALLBACK)) {
              // 400 kHz, same as the nRF5 SDK TWI config
    LOGF("LSM6 OK at 0x%02X\n", imuAddr);
    return true;
  }
  return false;
}

static void sleepModeEnter() {
  LOGF("Going to sleep (System OFF)...\n");
  led(PIN_LED_2, true);
  led(PIN_LED_1, false);

  // Wake on any button press (buttons are active LOW)
  for (int i = 0; i < 6; i++) {
    nrf_gpio_cfg_sense_input(g_ADigitalPinMap[button_pins[i]],
                             NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  }

  if (hasLSM6 && imu) {
    imu->writeRegister(0x10, 0x00);  // CTRL1_XL: accel power down
    imu->writeRegister(0x11, 0x00);  // CTRL2_G:  gyro power down
  }

  delay(2000);
  led(PIN_LED_1, false);
  led(PIN_LED_2, false);
  sd_power_system_off();
  while (true) { }       
}

void connect_callback(uint16_t conn_hdl) {
  LOGF("Connected\n");
  advCountToSleep = 0;
  isConnected = true;
  led(PIN_LED_1, false);
  led(PIN_LED_2, false);
}

void disconnect_callback(uint16_t conn_hdl, uint8_t reason) {
  LOGF("Disconnected (0x%02X)\n", reason);
  advCountToSleep = 0;
  rebeginCycles = 50;
  isConnected = false;
  received_config_integer = false;
  config_integer = 0xffff;
  led(PIN_LED_1, true);
}

void config_write_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  if (len == sizeof(int32_t)) {
    int32_t v;
    memcpy(&v, data, sizeof(v));
    config_integer = (uint16_t)(int16_t)v;
    received_config_integer = true;
    //Turn off both
    led(PIN_LED_2, false);
    led(PIN_LED_1, false);
    LOGF("Received config integer: %ld\n", (long)v);
  }
}

static void tick() {
  if (isConnected) {
    // LED feedback
    if (received_config_integer) {
      int pin = (config_integer == 0) ? PIN_LED_1 : PIN_LED_2;
      led(pin, advCountToSleep % 50 == 0);
    } else {
      if (advCountToSleep % 10 == 0) {
        led(cycleLed, true);
        canCycle = true;
      } else {
        led(cycleLed, false);
        if (canCycle) {
          canCycle = false;
          cycleLed = (cycleLed == PIN_LED_1) ? PIN_LED_2 : PIN_LED_1;
        }
      }
    }
    advCountToSleep++;
    if (hasLSM6 && imu) {
      AccValue[0] = imu->readRawAccelX();
      AccValue[1] = imu->readRawAccelY();
      AccValue[2] = imu->readRawAccelZ();
      AccValue[3] = imu->readRawGyroX();
      AccValue[4] = imu->readRawGyroY();
      AccValue[5] = imu->readRawGyroZ();
      int16_t t = 0;
      imu->readRegisterInt16(&t, 0x20); 
      AccValue[6] = t;
    } else {
      for (int i = 0; i < 7; i++) AccValue[i] = 0;
    }

    // Bytes 14..20: config byte + 6 button states
    AccValue[7] = AccValue[8] = AccValue[9] = AccValue[10] = 0;
    uint8_t* aux = (uint8_t*)&AccValue[7];
    aux[0] = (uint8_t)config_integer;
    for (int i = 0; i < 6; i++) {
      aux[i + 1] = (digitalRead(button_pins[i]) == LOW) ? 1 : 0;
    }

    if (rebeginCycles > 0) {
      rebeginCycles--;
    } else if (received_config_integer && memsChar.notifyEnabled()) {
      memsChar.notify((uint8_t*)AccValue, sizeof(int16_t) * STREAM_SIZE);
    }
  } else {
    advCountToSleep++;

    if (advCountToSleep < 1995 && advCountToSleep % 20 == 0) {
      LOGF("Count: %ld\n", (long)advCountToSleep);
    }
    if (advCountToSleep >= SLEEP_TICKS) {
      sleepModeEnter();
    }
  }
}

void setup() {
  LOG_BEGIN();
  LOGF("Proto paw starting...\n");

  pinMode(PIN_LED_1, OUTPUT);
  pinMode(PIN_LED_2, OUTPUT);
  led(PIN_LED_1, false);
  led(PIN_LED_2, false);
  for (int i = 0; i < 6; i++) pinMode(button_pins[i], INPUT_PULLUP);
  led(PIN_LED_2, true);  delay(50);
  led(PIN_LED_1, true);  delay(50);
  led(PIN_LED_2, false); delay(50);
  led(PIN_LED_1, false); delay(500);

#ifdef CUSTOM_PINS
  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
#endif
  Wire.setClock(400000);

  hasLSM6 = initLSM();

  if (!hasLSM6) {
    LOGF("LSM6 initialization failed!!!\n");
    for (int i = 0; i < 4; i++) {
      led(PIN_LED_1, true);  led(PIN_LED_2, true);  delay(250);
      led(PIN_LED_1, false); led(PIN_LED_2, false); delay(250);
    }
  }

  Bluefruit.autoConnLed(false);
  Bluefruit.configPrphConn(64, BLE_GAP_EVENT_LENGTH_DEFAULT, 16, 16);  // MTU 64 so 22 bytes fit
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.Periph.setConnInterval(6, 12);    // 7.5-15 ms, fine for 20 Hz
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
   led(PIN_LED_1, true);  led(PIN_LED_2, true);

  svc.begin();

  configChar.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  configChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  configChar.setFixedLen(sizeof(int32_t));
  configChar.setWriteCallback(config_write_callback);
  configChar.begin();
  configChar.write32(0);

  memsChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  memsChar.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  memsChar.setFixedLen(sizeof(int16_t) * STREAM_SIZE);
  memsChar.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addName();
  Bluefruit.ScanResponse.addService(svc);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(64, 64);   // 40 ms
  Bluefruit.Advertising.start(0);
  led(PIN_LED_2, false); 
  led(PIN_LED_1, true); 
  LOGF("Proto paw started, advertising\n");
}

// ---------------- Loop ----------------
void loop() {
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last >= TICK_MS) {
    last += TICK_MS;
    if (now - last > TICK_MS) last = now;      // resync if we fell behind
    tick();
  }
  delay(1);
}