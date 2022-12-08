
#include <Arduino.h>
#include <Wire.h>
#include <IridiumSBD.h>

#define NUBC_5V_ENABLE          5

#define NumDS28AE00s 10
#define PackedBytesLength NumDS28AE00s * 2 * 3/4

byte rawTempValues[NumDS28AE00s*2];
byte packedTempValues[PackedBytesLength];

bool readingUpperString;

void setup() {
  
  Wire.begin();                                                             
  Serial.begin(9600);
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  digitalWrite(NUBC_5V_ENABLE, HIGH); 
  
}

void loop(){

  delay(5000);
  Serial.println("CALLING READ UPPER TEMPERATURE STRING");
  readUpperTempString(); 
  delay(10000);
}


void readUpperTempString(){
    int command = 1;
    Serial.println("");
    Serial.println("Reading Upper Temperature String");
    Wire.beginTransmission(1);     
    Serial.println("You made it here");                                           
    Wire.write(command);   
    Serial.println("You made it here 2");                                                   
    int error = Wire.endTransmission(); 
    Serial.println("I BET YOU DIDN'T MAKE IT HERE"); 
    Serial.println("WIRE ERROR");
    Serial.print(error);
    delay(4000); 
    Serial.print("Fetching data...");
    Wire.requestFrom(1,160, false); // request 160 bytes
    
    int k = 0;
    Serial.println("");
    Serial.println("Raw Temperature Values: ");
    while (Wire.available()) {
      char c = Wire.read();
      rawTempValues[k] = c;
      k++;
      Serial.print(c, BIN);
      Serial.println(" ");
    }

    delay(1000);
    readingUpperString = false;
}
