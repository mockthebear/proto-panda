#pragma once

#include <Arduino.h>
#include <Wire.h>

#define LSM6DS3_ADDRESS            0x6A

#define LSM6DS3_WHO_AM_I_REG       0X0F
#define LSM6DS3_CTRL1_XL           0X10
#define LSM6DS3_CTRL2_G            0X11

#define LSM6DS3_STATUS_REG         0X1E

#define LSM6DS3_CTRL6_C            0X15
#define LSM6DS3_CTRL7_G            0X16
#define LSM6DS3_CTRL8_XL           0X17

#define LSM6DS3_OUT_TEMP_L         0X20

#define LSM6DS3_OUTX_L_G           0X22
#define LSM6DS3_OUTX_H_G           0X23
#define LSM6DS3_OUTY_L_G           0X24
#define LSM6DS3_OUTY_H_G           0X25
#define LSM6DS3_OUTZ_L_G           0X26
#define LSM6DS3_OUTZ_H_G           0X27

#define LSM6DS3_OUTX_L_XL          0X28
#define LSM6DS3_OUTX_H_XL          0X29
#define LSM6DS3_OUTY_L_XL          0X2A
#define LSM6DS3_OUTY_H_XL          0X2B
#define LSM6DS3_OUTZ_L_XL          0X2C
#define LSM6DS3_OUTZ_H_XL          0X2D



class LSM6DS3 {
  public:
    LSM6DS3(TwoWire& wire, uint8_t slaveAddress);
    virtual ~LSM6DS3();

    int begin();
    void end();

    // Accelerometer
    virtual int readAcceleration(float& x, float& y, float& z); // Results are in g (earth gravity).
    virtual float accelerationSampleRate(); // Sampling rate of the sensor.
    virtual int accelerationAvailable(); // Check for available data from accelerometer

    // Gyroscope
    virtual int readGyroscope(float& x, float& y, float& z); // Results are in degrees/second.
    virtual float gyroscopeSampleRate(); // Sampling rate of the sensor.
    virtual int gyroscopeAvailable(); // Check for available data from gyroscope

    // Temperature Sensor
    virtual int readTemperature(float& t); // Results are in deg. C
    virtual float temperatureSampleRate(); // Sampling rate of the sensor.
    virtual int temperatureAvailable(); // Check for available data from temperature sensor


  protected:
    int readRegister(uint8_t address);
    int readRegisters(uint8_t address, uint8_t* data, size_t length);
    int writeRegister(uint8_t address, uint8_t value);


  private:
    TwoWire* _wire;
    uint8_t _slaveAddress;
    int _csPin;
    int _irqPin;
};
