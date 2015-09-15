/**********************************************************************
 * pHSensor Logger example
 * An example to collect data from a pH sensor conected to an analog adapter
 *  using the pHSensor class

 * version 0.1 ALPHA 15/09/2015
 * Author: Jaime García  @peninquen
 * Licence: Apache License Version 2.0.
 *
 **********************************************************************/

#include "pHSensor.h"
#include <EEPROM.h>


#define N_CALPH 2
#define S_CALPH sizeof(calibrationData);
#define EM_CALPH_OFFSET 0
#define EM_CALPH_END (EM_CALPH_OFFSET + (N_CALPH * S_CALPH))
#define EM_END       (EM_CALPH_END + 1)
#define MAGIC_NUMBER 123 // byte number to check valid values in EEPROM

#define REFRESH_INTERVAL  1000   // refresh time, 1 second
#define WRITE_INTERVAL 5000      // values send to serial port, 5 seconds (5 * 1000)
#define PH_PIN A0                //

pHSensor pH; // instance to collect data
//variables to process and send values
boolean firstData;
float pHvalue;
float pHavg;
float pHmax;
float pHmin;
int counter = 0;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;


void setup() {
  Serial.begin(9600);
  delay(1000);
  pH.begin(PH_PIN, REFRESH_INTERVAL, EM_CALPH_OFFSET, 25.0);
  Serial.println("time(s), average pH, max pH, min pH");

  firstData = false;
}

void loop() {
  if (pH.available()) {
    pHvalue = pH.read();
    pHavg += pHvalue;
    counter++;
    if (!firstData) {
      if (pHmax < pHvalue) pHmax = pHvalue;
      if (pHmin > pHvalue) pHmin = pHvalue;
    }
    else {
      pHmax = pHvalue;
      pHmin = pHvalue;
      firstData = false;
    }
  }

  currentMillis = millis();
  if (currentMillis - previousMillis >= WRITE_INTERVAL) {
    previousMillis = currentMillis;
    firstData = true;
    pHavg /= counter;
    counter = 0;

    Serial.print(currentMillis / 1000);
    Serial.print(",");
    Serial.print(pHvalue);
    Serial.print(",");
    Serial.print(pHmax);
    Serial.print(",");
    Serial.println(pHmin);

  }
}
