/**********************************************************************/
/* pHSensor library
/* Arduino library to control a pH sensor usin an analog adapter
/* the adapter signal must be conected to one of analog pins
/* version 0.1 ALPHA 15/09/2015 
/* Author: Jaime García  @peninquen
/* License:  Apache License Version 2.0.
/*
/**********************************************************************/

#ifndef pHSensor_h
#define pHSensor_h

#include "Arduino.h"
#include <EEPROM.h>

#define BUFFERSIZE 10
#define MAGIC_NUMBER 123 // bye number to check valid values in EEPROM


class pHSensor {
  public:
    //constructor
    pHSensor();

    // Setup instance variables
    void begin(int pHPin, unsigned int interval, byte calAddress, float temperature);

    // calibrate and write data on EEPROM
    void calibrate(byte calAddress);

    // write temperature value
    void writeTempC(float temperature);

    // check interval and update data, interval must be greater than loop cycle
    void refreshData();

    // read _counter value in defined units
    float read();

    // is a new data available?
    boolean available();

  private:
    int _pHPin;                        // Analog pin conected to pH sensor adapter
    boolean _flag;                        // true when data is available, false when data is readed
    unsigned long _processTime;        // last time process
    unsigned int _interval;            // time [miliseconds] between adquisition data
    unsigned int _rawData[BUFFERSIZE]; // raw data from ADC
    unsigned int _index;
    float _temperature;                // temperature adjust??
    float _calRef1;                    // Calibration reference nº 1
    unsigned int _calRaw1;             // Calibration raw value nº 1
    float _calRef2;                    // Calibration reference nº 2
    unsigned int _calRaw2;             // Calibration reference nº 2
    float _offset;
    float _slope;
};

#endif
