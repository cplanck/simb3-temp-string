// this is test code for the SIMB3 main board to talk to the one-wire control Arduino over i2c. 
// it is used in conjunction with the temperaturestring-controller.ino sketch for the controller. 

// hardware setup includes an SIMB3 mainboard with a feather m0 plugged in through the aux i2c line. 
// note that battery power (17v) must be applied for the system to work. 

// Preliminary goal:
// Control the LED on the tempstring-meatball by writing 'On' and 'Off' commands


// General flow:
// For sending data to peripheral, simply run a Wire.beginTransmission, Wire.write(your data), and Wire.endTransmission. 
// On the peripheral side, this triggers an onRecieve() event which calls a function that can unpack the data sent from the master

#include <Wire.h>                                                        
String readString;                                                          
byte command;   

#define NUBC_5V_ENABLE          5

void setup() {
  Wire.begin();                                                             
  Serial.begin(9600);
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  digitalWrite(NUBC_5V_ENABLE, HIGH); 
}

void loop() {
  
  while (Serial.available() == 0) {}        //wait for data available
  String readString = Serial.readString();  
  readString.trim();                        // remove any \r \n whitespace at the end of the String

  if (readString == "On")
  {
    command = 1;
    Serial.print("TURNING LED ON...");
  }
  else if (readString == "Off")
  {
    command = 0;
    Serial.print("TURNING LED OFF...");

  }
  else if (readString == "Hi"){
    command = 2;
  }

  else if (readString == "it me"){
    command = 3;
  }

  else if (readString == "Hex"){
    command = 0x55;
  }

    else if (readString == "Fetch")
  {
    Serial.print("Fetching data...");
    Wire.requestFrom(1,50);
    while (Wire.available()) {
    char c = Wire.read();
    Serial.print(c);
    }
  }

  
  if (readString.length() > 0)                                            
  {                                
    readString = "";                                                        
  }
  Wire.beginTransmission(1);                                                
  Wire.write(command);                                                    
  Wire.endTransmission();    
}
