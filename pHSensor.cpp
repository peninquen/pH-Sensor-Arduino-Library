/**********************************************************************
 * pHSensor library
 * Arduino library to control a pH sensor usin an analog adapter
 * version 0.2 BETA 15/09/2015
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

void pHSensor::begin(int pHPin, unsigned int interval, byte calAddress) {
  _pHPin = pHPin;
  pinMode(_pHPin, INPUT);
  _interval = interval;
  _flag = false;
  for (int i = 0; i < BUFFERSIZE; i++) _rawData[i] = 0;
  _index = 0;
  EEPROM.get(calAddress, _cal1);
  if (_cal1.magicNumber != MAGIC_NUMBER) calibrate(calAddress, _cal1);
  calAddress += sizeof(calibration);
  EEPROM.get(calAddress, _cal2);
  if (_cal2.magicNumber != MAGIC_NUMBER) calibrate(calAddress, _cal2);
  _offset = _cal1.reference; //7.00
  _slope = (float)(_cal2.reference - _cal1.reference) / (_cal2.rawData - _cal1.rawData)  ;
  _processTime = millis();       // start timer

  Serial.print("pH Pin:"); Serial.print(_pHPin);
  Serial.print("  EEPROM address:"); Serial.println(calAddress);
}

/**************************************************************************/
/*calibrate sensor*/
calibration &pHSensor::calibrate(byte calAddress, calibration &cal) {
  int i;
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
    while (!_flag) {
      while (!refreshData());
      if (_index)
        i = BUFFERSIZE;
      else
        i = _index - 1;
      rawDev += (data - _rawData[i]) ^ 2;
    }
    Dev += (data - read()) ^ 2;
  }
  StDev = sqrt(Dev >> 6);
  rawStDev = sqrt(rawDev / BUFFERSIZE / 64);
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
// next, using calculated offset and slope make a linear transformation
short pHSensor::read() {
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
  _flag = false;
  return (_offset + _slope * (_rawData[BUFFERSIZE / 2] - _cal1.rawData));
}

/***************************************************************************/
/* Is new data available */

boolean pHSensor::available() {
  refreshData();
  return _flag;
}
