// Basic development code to read a Maxim Integrated DS18B20 digital temperature sensor using the one-wire protocol

// Hardware: an Feather M0 with a DS18B20 powered and grounded with the data line connected to pin 13. 

// Written 4 December, 2022 by Cameron Planck

#include <Wire.h>       
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire  at(13);  //  on pin 13 (a 4.7K (ish) resistor is necessary, it wont work without it)

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature air_temp(&at);


void setup(){
  Serial.begin(9600);
  air_temp.begin();
}

void loop(){
  // use the Dallas Temperature library to issue the temperature request command
  air_temp.requestTemperatures();

  // reset one-wire bus
  at.reset(); 
 
  Serial.println("Reading Scratchpad...");

  // skip ROM
  at.write(0xCC); 

  // read scratchpad command
  at.write(0xBE); 

  // step first two bytes, write them to scratchpad
  byte scratchpad[8];
  for (int i = 0; i < 2; i++) { 
    scratchpad[i] = at.read();
    Serial.print(scratchpad[i], BIN);
    Serial.print(" ");
  }
  
 float temp = air_temp.getTempCByIndex(0);
  Serial.println(temp); 
  Serial.println("");
}
