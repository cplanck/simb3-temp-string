// this is test code for the SIMB3 main board to talk to the one-wire control Arduino over i2c. 

// hardware setup includes an SIMB3 mainboard with a feather m0 plugged in through the aux i2c line. 
// note that battery power (17v) must be applied for the system to work. 

// Preliminary goal:
// Control the LED on the tempstring-meatball by writing 'On' and 'Off' commands


#include <Wire.h>                                                        
String readString;                                                          
byte I2C_OnOff;   

#define NUBC_5V_ENABLE          5

void setup() {
  Wire.begin();                                                             
  Serial.begin(9600);
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  digitalWrite(NUBC_5V_ENABLE, HIGH); 
}

void loop() {
  
  while (Serial.available() == 0) {}        //wait for data available
  String readString = Serial.readString();  //read until timeout
  readString.trim();                        // remove any \r \n whitespace at the end of the String

  if (readString == "On")
  {
    I2C_OnOff = 1;
    Serial.print("TURNING LED ON...");
  }
  else if (readString == "Off")
  {
    I2C_OnOff = 0;
    Serial.print("TURNING LED OFF...");

  }
  if (readString.length() > 0)                                            
  {                                
    readString = "";                                                        
  }
  Wire.beginTransmission(1);                                                
  Wire.write(I2C_OnOff);                                                    
  Wire.endTransmission();    
}
