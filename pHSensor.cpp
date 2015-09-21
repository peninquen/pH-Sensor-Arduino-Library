/**********************************************************************
 * pHSensor library
 * Arduino library to control a pH sensor usin an analog adapter
 * version 0.2 ALPHA 15/09/2015
 * Author: Jaime García  @peninquen
 * License: Apache License Version 2.0.
 *
 ********************************************************************
  */
#include "pHSensor.h"

/***************************************************************************/
/*constructor*/

pHSensor::pHSensor() {
}

/***************************************************************************/
/*Setup variables and initialize interrupts*/

void pHSensor::begin(int pHPin, unsigned short interval, byte calAddress) {
  _pHPin = pHPin;
  pinMode(_pHPin, INPUT);  // analog input pin
  _flag = false;
  for (int i = 0; i < BUFFERSIZE; i++) _rawData[i] = 0;
  _index = 0;
  EEPROM.get(calAddress, _cal1);
  if (_cal1.magicNumber != MAGIC_NUMBER) calibrate(calAddress, _cal1);
  calAddress += sizeof(calibration);
  EEPROM.get(calAddress, _cal2);
  if (_cal2.magicNumber != MAGIC_NUMBER) calibrate(calAddress, _cal2);
  _slope = (float)(_cal2.reference - _cal1.reference) / (_cal2.rawData - _cal1.rawData);
  _offset = _cal1.reference - _slope * _cal1.rawData;
  _processTime = millis();       // start timer
  _interval = interval; //interval during loop function

  Serial.print("pH Pin:"); Serial.print(_pHPin);
  Serial.print("  EEPROM address:"); Serial.println(calAddress);
}

/**************************************************************************/
/*calibrate sensor
  first calculate average value of 64 median values;
  second, use average to calculate standard deviation of the filtered and unfiltered readings*/
calibration &pHSensor::calibrate(byte calAddress, calibration &cal) {
  unsigned short data;
  unsigned short StDev = 0;    //standard deviation of filtered data
  unsigned short rawStDev = 0; //standard deviation of unfiltered data
  unsigned long Dev = 0;       // squared deviation of filtered data
  unsigned long rawDev = 0;    // squared deviation of unfiltered data

  Serial.print("intro ref:");
  cal.reference = Serial.parseInt();
  for (int count = 64; count; --count) {
    while (!available()); // wait until data[] is full
    data += read();
  }
  cal.rawData = data >> 6;  //average value of 64 median values
  Serial.print("ADC value:");
  Serial.println(cal.rawData);

  for (int count = 64; count; --count) {
    for (int index = BUFFERSIZE; index; --index) {
      _rawData[index] = analogRead(_pHPin);
      rawDev += (cal.rawData - _rawData[_index]) ^ 2;
    }
    for (int i = 0; i < BUFFERSIZE / 2; i++) {
      for (int j = i; j < BUFFERSIZE; j++) {
        if (_rawData[i] > _rawData[j]) {
          data = _rawData[i];
          _rawData[i] = _rawData[j];
          _rawData[j] = data;
        }
      }
    }
    Dev += (cal.rawData - _rawData[BUFFERSIZE / 2]) ^ 2;
  }
  StDev = sqrt(Dev >> 6);
  rawStDev = sqrt(rawDev / BUFFERSIZE / 64);
  Serial.print("filtered standard deviation:");
  Serial.println(StDev);
  Serial.print("un-filtered standard deviation:");
  Serial.println(rawStDev);
}

/***************************************************************************/
/*check interval and read data, interval must be greater than loop cycle*/
boolean pHSensor::refreshData() {
  unsigned long nowTime = millis();
  if (nowTime - _processTime >= _interval) {
    _rawData[_index] = analogRead(_pHPin);
    _processTime = nowTime;     //stamp process time
    if (++_index = BUFFERSIZE) {
      _flag = true;
      _index = 0;
    }
    return true;
  }
  return false;
}

/***************************************************************************/
/*read sensor value*/
// first, sort rawData array to get median value, rawData[BUFFERSIZE/2]
// next, make a linear transformation using calculated offset and slope
short pHSensor::read() {
  unsigned short data;
  for (int i = 0; i < BUFFERSIZE / 2; i++) {
    for (int j = i; j < BUFFERSIZE; j++) {
      if (_rawData[i] > _rawData[j]) {
        data = _rawData[i];
        _rawData[i] = _rawData[j];
        _rawData[j] = data;
      }
    }
  }
  _flag = false;
  return (_offset + _slope * _rawData[BUFFERSIZE / 2]);
}

/***************************************************************************/
/* Is new data available */

boolean pHSensor::available() {
  refreshData();
  return _flag;
}
