// Test code for verifying basic communication between the SIMB3 brain and one-wire controller devices.
// This is mainboard code. It's used in conjunction with one-wire-controller-production.ino.
// Note: after performing a ROM search command, remove the jumper and let the code run again to see temps.
// Written 25 January 2023 by Cameron Planck

#include <Arduino.h>
#include <Wire.h>

#include "SIMB3_Onewire_Controller.h"

#define SIMB3_5V_ENABLE          5

SIMB3_Onewire_Controller SIMB3_ONEWIRE_CONTROLLER;

byte topTempStringBuffer[160];
byte topTempStringPackedBuffer[120];
byte bottomTempStringBuffer[160];
byte bottomTempStringPackedBuffer[120];
byte airTemperatureBuffer[2];

#define TEMPSTRING_TIMEOUT       10000 //10 seconds
#define AIR_TEMP_TIMEOUT         10000 //10 seconds

void setup()
{

  Wire.begin(); // Join the I2C bus as master
  pinMode(SIMB3_5V_ENABLE, OUTPUT);

}

void loop()
{
 
  readOnewireController();
  
};


void readOnewireController(){

  Serial.print(F("Resetting One-Wire controller..."));
  SIMB3_ONEWIRE_CONTROLLER.reset();
  Serial.println(F("Controller reset.\n"));

  // Read top temperature string
  bool done = false;
  unsigned long timeout = millis() + TEMPSTRING_TIMEOUT;
  while(!done && (millis() < timeout)){
    done = SIMB3_ONEWIRE_CONTROLLER.readTopString();
  }

  delay(2000);

  SIMB3_ONEWIRE_CONTROLLER.requestTemperatureStringBytes(1, topTempStringBuffer);


  // Read bottom temperature string
  done = false;
  timeout = millis() + TEMPSTRING_TIMEOUT;
  while(!done && (millis() < timeout)){
    done = SIMB3_ONEWIRE_CONTROLLER.readBottomString();
  }
  
  delay(2000);
  
  SIMB3_ONEWIRE_CONTROLLER.requestTemperatureStringBytes(2, bottomTempStringBuffer);

  // Read air temperature
  // done = false;
  // timeout = millis() + AIR_TEMP_TIMEOUT;
  // while(!done && (millis() < timeout)){
  //   done = SIMB3_ONEWIRE_CONTROLLER.readAirTemperature();
  // }

  delay(2000);

  SIMB3_ONEWIRE_CONTROLLER.requestAirTempBytes(airTemperatureBuffer);

  Serial.println(F("Top Temperature String(Deg C)"));
  SIMB3_ONEWIRE_CONTROLLER.printTemperatureString(1, topTempStringBuffer);

  Serial.println(F(""));
  Serial.println(F("Bottom Temperature String(Deg C)"));
  SIMB3_ONEWIRE_CONTROLLER.printTemperatureString(2, bottomTempStringBuffer);

//   printString(F("Air Temperature"));
//  SIMB3_ONEWIRE_CONTROLLER.decodeTemperatureBytes(airTemp[0], airTemp[1]);
//   Serial.println();


  
}