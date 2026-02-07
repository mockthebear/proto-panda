/*
    Proto paw controller

    Wiring: 
    Connect two leds, one on pin 6 and one on pin 7. Positive on pins and the other terminal to GND

    Connect 5 or 6 buttons. 
    Each on pin 8 to 13. The other terminal goes to GND

    If in hand, connect an accelerometer LSM6DS3TR
    SLC on pin 2
    SDA on pin 3
    VCC on 3.3v
    GND on GND

    On arduino IDE, select board GEEKBLE nano esp32-s3 if you're using esp32-s3 super mini
*/

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Arduino_LSM6DS3.h>
#include <Wire.h>

#define PIN_SLC 2
#define PIN_SDA 3
#define PIN_BUTTON_1 8
#define PIN_BUTTON_2 9
#define PIN_BUTTON_3 10
#define PIN_BUTTON_4 11
#define PIN_BUTTON_5 12
#define PIN_BUTTON_6 13
#define LED_1 6
#define LED_2 7

static NimBLEServer* pServer;
#define STREAM_SIZE 11
#define BUTTONS 6
static int16_t AccValue[STREAM_SIZE];

// Define the UUIDs
#define SERVICE_UUID "d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1"
#define CHARACTERISTIC_RW_UUID "d4d3fafb-c4c1-c2c3-b4b3-b2b1a4a3a2a1"
#define CHARACTERISTIC_NOTIFY_UUID "d4d3afaf-c4c1-c2c3-b4b3-b2b1a4a3a2a1"


NimBLEAdvertising* pAdvertising = nullptr;

int button_states[BUTTONS] = {0};
int myId = -1;
bool connected = false;
bool hasIMU = false;
unsigned long lastNotifyTime = 0;
unsigned long connectedBlinkCounter = 0;


LSM6DS3Class *imuObj;

const uint8_t button_pins[] = {
    PIN_BUTTON_1,
    PIN_BUTTON_2,
    PIN_BUTTON_3,
    PIN_BUTTON_4,
    PIN_BUTTON_5,
    PIN_BUTTON_6,
};


void updateInputData(){
    if (hasIMU){
        float x,y,z,gx,gy,gz,t;
        imuObj->readAcceleration(x, y, z);
        imuObj->readGyroscope(gx, gy, gz);
        imuObj->readTemperature(t);
        //We want the raw data, but the lib does not give it to us >:(
        AccValue[0] = x  * 32768.0/4.0;
        AccValue[1] = y  * 32768.0/4.0;
        AccValue[2] = z  * 32768.0/4.0;
        AccValue[3] = gx  * 32768.0/4.0;
        AccValue[4] = gy  * 32768.0/4.0;
        AccValue[5] = gz  * 32768.0/4.0;
        AccValue[6] = (y  / 16.0)-25;
    }
    uint8_t* aux = (uint8_t*)&AccValue[7];
    aux[0] = (uint8_t)myId;
    for (int i = 0; i < BUTTONS; i++) {
        // Buttons are active LOW due to pullup
        button_states[i] = (digitalRead(button_pins[i]) == LOW) ? 1 : 0;
        aux[i + 1] = button_states[i];
    }
}

void turnOnConnectedLed(){
    if (myId%2 == 0){
        digitalWrite(LED_2, HIGH);
    }else{
        digitalWrite(LED_2, LOW);
    }
    if (myId%2 == 1){
        digitalWrite(LED_1, HIGH);
    }else{
        digitalWrite(LED_1, LOW);
    }
}

// Callback classes
class RWCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
public:
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        pCharacteristic->setValue(myId);
    }

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        // Read the new integer value
        std::string value = pCharacteristic->getValue();
        if (value.length() == sizeof(int)) {
            myId = *reinterpret_cast<const int*>(value.data());
            Serial.printf("My controller id is: %d\n", myId);
            turnOnConnectedLed();
        } 
    }
};

class NotifyCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
public:
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        updateInputData(); 
        pCharacteristic->setValue((uint8_t*)AccValue, sizeof(AccValue));
    }
    
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        Serial.println("Client subscribed to notifications");
    }
};

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        Serial.println("Client connected");
        Serial.print("Client address: ");
        Serial.println(connInfo.getAddress().toString().c_str());
        myId = -1;
        connected = true;
        pAdvertising->stop();
        digitalWrite(LED_1, LOW);
        digitalWrite(LED_2, LOW);
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
        Serial.print("Client disconnected - reason: ");
        Serial.println(reason);
        myId = -1;
        connected = false;
        pAdvertising->start();
        digitalWrite(LED_1, HIGH);
        digitalWrite(LED_2, LOW);
    }
};

// Create callback instances
RWCharacteristicCallbacks rwCallbacks;
NotifyCharacteristicCallbacks notifyCallbacks;

void setup(void) {
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);

    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    for (int i = 0; i < BUTTONS; i++) {
        pinMode(button_pins[i], INPUT_PULLUP);
    }
    Serial.begin(115200);
    Serial.println("Starting Proto Panda Hand 0xc2 BLE Server");

    /** Initialize NimBLE and set the device name */
    NimBLEDevice::init("Proto panda hand 0xc2");

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    NimBLECharacteristic* pRWCharacteristic = pService->createCharacteristic(CHARACTERISTIC_RW_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pRWCharacteristic->setCallbacks(&rwCallbacks);


    NimBLECharacteristic* pNotifyCharacteristic = pService->createCharacteristic(CHARACTERISTIC_NOTIFY_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pNotifyCharacteristic->setCallbacks(&notifyCallbacks);

    memset(AccValue, 0, sizeof(AccValue));

    pService->start();

    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->enableScanResponse(true);
    pAdvertising->setName("Proto panda hand 0xc2");
    Wire.begin(PIN_SDA,PIN_SLC);
    imuObj = new LSM6DS3Class(Wire, 0x6A);
    Wire.begin(PIN_SDA,PIN_SLC);
    if (!imuObj->begin()) {
        imuObj = new LSM6DS3Class(Wire, 0x6B);
        if (!imuObj->begin()) {
            Serial.println("Failed to initialize IMU!");
            for (int i=0;i<8;i++){
                digitalWrite(LED_1, HIGH);
                digitalWrite(LED_2, HIGH);
                delay(250);
                digitalWrite(LED_1, LOW);
                digitalWrite(LED_2, LOW);
                delay(250);
            }
        }else{
            hasIMU = true;
        }
    }else{
        hasIMU = true;
    }


    delay(100);
    digitalWrite(LED_1, HIGH);
    delay(250);
    digitalWrite(LED_2, HIGH);
    delay(250);
    digitalWrite(LED_1, LOW);
    delay(250);
    digitalWrite(LED_2, LOW);
    delay(250);

    pAdvertising->start();

    Serial.println("Advertising Started");
    Serial.println("Device name: Proto panda hand 0xc1");
    Serial.println("Service UUID: " SERVICE_UUID);
    digitalWrite(LED_1, HIGH);
}


void loop() {
    if (millis() - lastNotifyTime >= 50) {
        if (pServer->getConnectedCount()) {
            if (myId != -1){
                NimBLEService* pSvc = pServer->getServiceByUUID(SERVICE_UUID);
                if (pSvc) {
                    NimBLECharacteristic* pChr = pSvc->getCharacteristic(CHARACTERISTIC_NOTIFY_UUID);
                    if (pChr) {
                        digitalWrite(LED_1, LOW);
                        digitalWrite(LED_2, LOW);
                        if (connectedBlinkCounter < millis()){
                            connectedBlinkCounter = millis()+500;
                            turnOnConnectedLed();
                        }
                        updateInputData();  
                        // Send notification
                        pChr->setValue((uint8_t*)AccValue, sizeof(AccValue));
                        pChr->notify();
                    }
                }
            }else{
                static int ledMode = 0;
                if (ledMode == 0){
                    digitalWrite(LED_1, HIGH);
                }else if(ledMode == 1){
                    digitalWrite(LED_1, LOW);
                }else if(ledMode == 4){
                    digitalWrite(LED_2, HIGH);
                }else if(ledMode == 5){
                    digitalWrite(LED_2, LOW);
                }else if(ledMode == 7){
                    ledMode = -1;
                }
                ledMode++;
            }
        }
        lastNotifyTime = millis();
    }
    
    delay(5); 
}