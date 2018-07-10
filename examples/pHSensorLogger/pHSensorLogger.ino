/**********************************************************************
 * pHSensor Logger example
 * An example to collect data from a pH sensor conected to an analog adapter
 *  using the pHSensor class

 * version 0.2 ALPHA 16/09/2015
 * Author: Jaime García  @peninquen
 * License: Apache License Version 2.0.
 *
 **********************************************************************/

#include "pHSensor.h"
#include <EEPROM.h>


#define N_CALPH 2
#define S_CALPH sizeof(calibration);
#define EM_CALPH_OFFSET 0
#define EM_CALPH_END (EM_CALPH_OFFSET + (N_CALPH * S_CALPH))
#define EM_END       (EM_CALPH_END + 1)

#define REFRESH_INTERVAL  1000   // refresh time, 1 second
#define WRITE_INTERVAL 5000      // values send to serial port, 5 seconds (5 * 1000)
#define PH_PIN A0                //

pHSensor pH; // instance to process data
//variables to process and send values
boolean firstData;
short pHvalue;
short pHmax;
short pHmin;
long pHavg;
int counter = 0;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;


void setup() {
  Serial.begin(9600);
  delay(1000);
  pH.begin(PH_PIN, REFRESH_INTERVAL, EM_CALPH_OFFSET);
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
    Serial.print((float)pHvalue / 100);
    Serial.print(",");
    Serial.print((float)pHmax / 100);
    Serial.print(",");
    Serial.println((float)pHmin / 100);

  }
}
