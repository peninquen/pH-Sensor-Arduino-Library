/**********************************************************************/
/* pHSensor library
/* Arduino library to control a pH sensor using an analog adapter
/* the adapter signal must be conected to one of the analog pins
/* version 0.2 ALPHA 16/09/2015 
/* Author: Jaime García  @peninquen
/* License:  Apache License Version 2.0.
/*
/**********************************************************************/

#ifndef pHSensor_h
#define pHSensor_h

#include "Arduino.h"
#include <EEPROM.h>

#define BUFFERSIZE 10
#define MAGIC_NUMBER 1234 // bye number to check valid values in EEPROM
struct calibration {
  short magicNumber;
  short reference;
  short rawData;
};


class pHSensor {
  public:
    //constructor
    pHSensor();

    // Setup instance variables
    void begin(int pHPin, unsigned int interval, byte calAddress);

    // calibrate and write data on EEPROM
    calibration &calibrate(byte calAddress, calibration &cal);

    // check interval and update data, interval must be greater than loop cycle
    boolean refreshData();

    // read _counter value in defined units
    short read();

    // is a new data available?
    boolean available();

  private:
    int _pHPin;                        // Analog pin conected to pH sensor adapter
    boolean _flag;                        // true when data is available, false when data is readed
    unsigned long _processTime;        // last time process
    unsigned int _interval;            // time [miliseconds] between adquisition data
    unsigned int _rawData[BUFFERSIZE]; // raw data from ADC
    unsigned int _index;
    calibration _cal1;                    // Calibration reference nº 1
    //short _calRaw1;             // Calibration raw value nº 1
    calibration _cal2; //short _calRef2;                    // Calibration reference nº 2
    //short _calRaw2;             // Calibration reference nº 2
    short _offset;
    float _slope;
};

#endif
