// SIMB3 ONE-WIRE TEMPSTRING MAINBOARD CODE
// See: simb3-ow-tempstring-controller for corresponding controller development code. This sketch works in combination with simb3-ow-controller
// this development sketch is for streaming data from multiple DS28EA00 temperature chips across i2c to the SIMB3 mainboard.
// enter the following commands into the serial terminal to request data from the SIMB3 one-wire controller 
// AVAILABLE COMMANDS:
// request top temperature string: READ-TT
// request bottom temperature string: READ-TB
// request air temperature: READ-AT
// write top string ROMS to EEPROP: WRITE-TT-ROM
// write bottom string ROMS to EEPROP: WRITE-TB-ROM

// the temperature byte values are read off each chip and appended to an array which is streamed to the SIMB3 mainboard upon request. 
 
// the hardware configuration is an SIMB3 mainboard and Arduino Feather m0 connected over I2C (SDA and SCK). 
// the mainboard is USB connected and powered from a ~18 volt source. The control board is connected to the 5V I2C line via VBUS. 
// the Feather m0 is powered via 5v VBUS from the SIMB3 mainboard and also connected to the I2C lines via SDA and SCK 
// note that this sketch should work on full SIMB3 hardware. It can be used as a test sketch to validate the one-wire temp string. 

// written 1 December, 2022 by Cameron Planck


#include <Wire.h>                                                        
String readString;                                                          
byte command;   
byte receiveBuffer[160];

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

  if (readString == "READ-TT")
  {
    command = 1;
    Serial.println("");
    Serial.println("Reading Upper Temperature String");
    Wire.beginTransmission(1);                                                
    Wire.write(command);                                                    
    Wire.endTransmission();  
    Serial.print("Fetching data...");
    Wire.requestFrom(1,160); // request 160 bytes

    int k = 0;
    Serial.println("");
    while (Wire.available()) {
    char c = Wire.read();
    receiveBuffer[k] = c;
//    Serial.print(c, BIN); // print out binary response
//    Serial.print(" ");
    k++;
    }
    decodeTemps(160);
  }

  if (readString == "READ-AT")
  {
    command = 3;
    Serial.println("");
    Serial.println("Reading DS18B20 Air Temperature");
    Wire.beginTransmission(1);                                                
    Wire.write(command);                                                    
    Wire.endTransmission();  
    Serial.print("Fetching data...");
    Wire.requestFrom(1,2); // request 2 bytes

    int k = 0;
    Serial.println("");
    while (Wire.available()) {
    char c = Wire.read();
    receiveBuffer[k] = c;
    Serial.print(c, BIN); // print out binary response
    Serial.print(" ");
    k++;
    }

    decodeTemps(2);
    
  }
}

void decodeTemps(int numOfValues){
  byte tempBytes[2];
  int k = 1;
  Serial.println("");
  Serial.print("Results: ");
  for(int i = 0; i < numOfValues; i=i+2){
    tempBytes[0] = receiveBuffer[i];
    tempBytes[1] = receiveBuffer[i+1];
  
    int32_t neg = 0x0;
    if (tempBytes[1] & 0x80) // if the 
      neg = 0xFFF80000;
  
    // shift and stack bits in MSbyte and LSbyte to form combined binary number
    int32_t fpTemperature = 0;
    fpTemperature = (((int16_t) tempBytes[1]) << 11)
                    | (((int16_t) tempBytes[0]) << 3)
                    | neg;
  
    // manually calculate temperature from raw
    float convertedTemp = fpTemperature * 0.0078125f;
    Serial.print(convertedTemp);
    Serial.print(" ");

    if(i%5 == 0){
      Serial.println("");
      }
    k++;
  }
}
  
