// Basic I2C controller (master) code for iterative communication  
// written 4 December, 2022 by Cameron Planck

#include <Wire.h>                                                        
String readString;                                                          
byte command;   
byte receiveBuffer;

#define NUBC_5V_ENABLE          5

void setup() {
  Wire.begin();                                                             
  Serial.begin(9600);
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  digitalWrite(NUBC_5V_ENABLE, HIGH); 
}

void loop() {
    command = 2;
    Serial.println("");
    Serial.println("Master communicating to slave...");
    
    Wire.beginTransmission(1);                                                
    Wire.write(command);                                                    
    Wire.endTransmission();  
    Serial.println("End Transmission Complete");
    
    delay(2000); 
    Serial.print("Master requesting data from slave...");
    Wire.requestFrom(1,80); // request 160 bytes
    Serial.println("");
    
    while (Wire.available()) {
      char c = Wire.read();
//      receiveBuffer = c;
      Serial.print(c, BIN); // print out binary response
      Serial.print(" ");
  }
  delay(2000);
}

  
