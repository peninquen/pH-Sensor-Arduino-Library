/**********************************************************************
 * pHSensor library
 * Arduino library to control a pH sensor usin an analog adapter
 * version 0.1 ALPHA 15/09/2015
 * Author: Jaime García  @peninquen
 * Licence: Apache License Version 2.0.
 *
 ********************************************************************
  */
#include "pHSensor.h"

struct CalibrationData {
  byte magicNumber;
  float calRef;
  unsigned int calRaw;
} calibrationData;

/***************************************************************************/
/*constructor*/

pHSensor::pHSensor() {
}

/***************************************************************************/
/*Setup variables and initialize interrupts*/

void pHSensor::begin(int pHPin, unsigned int interval, byte calAddress, float temperature) {
  _pHPin = pHPin;
  _interval = interval;
  _temperature = temperature;
  _flag = false;
  for (int i = 0; i < BUFFERSIZE; i++) _rawData[i] = 0;
  _index = 0;
  CalibrationData calData;
  EEPROM.get(calAddress, calData);
  if (calData.magicNumber == MAGIC_NUMBER) {
    _calRef1 = calData.calRef;
    _calRaw1 = calData.calRaw;
  }
  else calibrate(calAddress);
  calAddress += sizeof(calData);
  EEPROM.get(calAddress, calData);
  if (calData.magicNumber == MAGIC_NUMBER) {
    _calRef2 = calData.calRef;
    _calRaw2 = calData.calRaw;
  }
  else calibrate(calAddress);
  _offset = _calRef1; //7.00
  _slope = (_calRef2 - _calRef1) / (_calRaw2 - _calRaw1)  ;
  _processTime = millis();       // start timer

  Serial.print("pH Pin:"); Serial.print(_pHPin);
  Serial.print("  EEPROM address:"); Serial.println(calAddress);
  pinMode(pHPin, INPUT); // conect external pull up resistor 10 Kohm on input pin
}

/**************************************************************************/
/*calibrate sensor*/
void pHSensor::calibrate(byte calAddress) {

}

/*****************************************************************************/
/* write temperature value */
    void pHSensor::writeTempC(float temperature){
      _temperature = temperature;
    }




/***************************************************************************/
/*check interval and update data, interval must be greater than loop cycle*/
void pHSensor::refreshData() {
  unsigned long nowTime = millis();
  if (nowTime - _processTime >= _interval) {
    _rawData[_index] = analogRead(_pHPin);
    _processTime = nowTime;     //stamp process time
    if (++_index = BUFFERSIZE) {
      _flag = true;
      _index = 0;
    }
  }
}

/***************************************************************************/
/*read sensor value*/
// first, sort rawData array to get median value, rawData[BUFFERSIZE/2]
// next, using calculated offset and slope make a linear transformation
// last, correct by temperature (if required)
float pHSensor::read() {
  unsigned int temporal;
  for (int i = 0; i < BUFFERSIZE / 2 - 1; i++) {
    for (int j = i; j < BUFFERSIZE; j++) {
      if (_rawData[i] > _rawData[j]) {
        temporal = _rawData[i];
        _rawData[i] = _rawData[j];
        _rawData[j] = temporal;
      }
    }
  }
  float pHValue = _offset + _slope * (_rawData[BUFFERSIZE / 2] - _calRaw1);
  if (_temperature!=25.0) pHValue = pHValue; // adjust temperature correction
  _flag = false;
  return pHValue;
}

/***************************************************************************/
/* Is new data available */

boolean pHSensor::available() {
  refreshData();
  return _flag;
}
