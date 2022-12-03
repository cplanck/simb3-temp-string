#include <Wire.h>       
#include <OneWire.h>
#include <DallasTemperature.h>

//OneWire  ds(12);  // on pin 12 (a 4.7K (ish) resistor is necessary, it wont work without it)
OneWire  at(13);  //  on pin 13 (a 4.7K (ish) resistor is necessary, it wont work without it)

// Pass our oneWire reference to Dallas Temperature sensor 
//DallasTemperature sensors(&ds);
DallasTemperature air_temp(&at);


void setup(){
  Serial.begin(9600);
  air_temp.begin();
}

void loop(){
  air_temp.requestTemperatures();

  at.reset(); // reset one-wire bus
 
  Serial.println("Reading Scratchpad...");
  at.write(0xCC); // skip ROM
  at.write(0xBE); // read scratchpad command
  byte scratchpad[8];
  for (int i = 0; i < 2; i++) { // step first two bytes, write them to scratchpad
    scratchpad[i] = at.read();
    Serial.print(scratchpad[i], BIN);
    Serial.print(" ");
  }
  
 float temp = air_temp.getTempCByIndex(0);
  Serial.println(temp); 
  Serial.println("");
}
