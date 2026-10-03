#include "tools/LSM6DS3.hpp"

LSM6DS3::LSM6DS3(TwoWire& wire, uint8_t slaveAddress) :
  _wire(&wire),
  _slaveAddress(slaveAddress)
{
}


LSM6DS3::~LSM6DS3()
{
}

int LSM6DS3::begin()
{

    _wire->begin();

  
  if (!(readRegister(LSM6DS3_WHO_AM_I_REG) == 0x6A || readRegister(LSM6DS3_WHO_AM_I_REG) == 0x68)) {
    end();
    return 0;
  }

  //set the gyroscope control register to work at 104 Hz, 2000 dps and in bypass mode
  writeRegister(LSM6DS3_CTRL2_G, 0x4C);

  // Set the Accelerometer control register to work at 104 Hz, 4 g,and in bypass mode and enable ODR/4
  // low pass filter (check figure9 of LSM6DS3's datasheet)
  writeRegister(LSM6DS3_CTRL1_XL, 0x4A);

  // set gyroscope power mode to high performance and bandwidth to 16 MHz
  writeRegister(LSM6DS3_CTRL7_G, 0x00);

  // Set the ODR config register to ODR/4
  writeRegister(LSM6DS3_CTRL8_XL, 0x09);

  return 1;
}

void LSM6DS3::end()
{

    writeRegister(LSM6DS3_CTRL2_G, 0x00);
    writeRegister(LSM6DS3_CTRL1_XL, 0x00);
    _wire->end();
  
}

int LSM6DS3::readAcceleration(float& x, float& y, float& z)
{
  int16_t data[3];

  if (!readRegisters(LSM6DS3_OUTX_L_XL, (uint8_t*)data, sizeof(data))) {
    x = NAN;
    y = NAN;
    z = NAN;

    return 0;
  }

  x = data[0] * 4.0 / 32768.0;
  y = data[1] * 4.0 / 32768.0;
  z = data[2] * 4.0 / 32768.0;

  return 1;
}

int LSM6DS3::accelerationAvailable()
{
  if (readRegister(LSM6DS3_STATUS_REG) & 0x01) {
    return 1;
  }

  return 0;
}

float LSM6DS3::accelerationSampleRate()
{
  return 104.0F;
}

int LSM6DS3::readGyroscope(float& x, float& y, float& z)
{
  int16_t data[3];

  if (!readRegisters(LSM6DS3_OUTX_L_G, (uint8_t*)data, sizeof(data))) {
    x = NAN;
    y = NAN;
    z = NAN;

    return 0;
  }

  x = data[0] * 2000.0 / 32768.0;
  y = data[1] * 2000.0 / 32768.0;
  z = data[2] * 2000.0 / 32768.0;

  return 1;
}

int LSM6DS3::gyroscopeAvailable()
{
  if (readRegister(LSM6DS3_STATUS_REG) & 0x02) {
    return 1;
  }

  return 0;
}

float LSM6DS3::gyroscopeSampleRate()
{
  return 104.0F;
}

int LSM6DS3::readTemperature(float& t)
{
  int16_t data[1];

  if (!readRegisters(LSM6DS3_OUT_TEMP_L, (uint8_t*)data, sizeof(data))) {
    t = NAN;

    return 0;
  }

  t = data[0] / 16.0 + 25;

  return 1;
}

int LSM6DS3::temperatureAvailable()
{
  if (readRegister(LSM6DS3_STATUS_REG) & 0x04) {
    return 1;
  }

  return 0;
}

float LSM6DS3::temperatureSampleRate()
{
  return 52.0F;
}

int LSM6DS3::readRegister(uint8_t address)
{
  uint8_t value;

  if (readRegisters(address, &value, sizeof(value)) != 1) {
    return -1;
  }

  return value;
}

int LSM6DS3::readRegisters(uint8_t address, uint8_t* data, size_t length)
{

    _wire->beginTransmission(_slaveAddress);
    _wire->write(address);

    if (_wire->endTransmission(false) != 0) {
      return -1;
    }

    if (_wire->requestFrom(_slaveAddress, length) != length) {
      return 0;
    }

    for (size_t i = 0; i < length; i++) {
      *data++ = _wire->read();
    }
  
  return 1;
}

int LSM6DS3::writeRegister(uint8_t address, uint8_t value)
{

    _wire->beginTransmission(_slaveAddress);
    _wire->write(address);
    _wire->write(value);
    if (_wire->endTransmission() != 0) {
      return 0;
    }
 
  return 1;
}

