// Development code to communicate and fetch data from the SIMB3 One-Wire Controller
// Test code for communicating with the One-Wire controller to fetch SIMB3 one-wire temperature chain and air temp data
// Data is then fetched over I2C in 32 byte chunks, assembled into an arrays, packed, converted, and printed to the console
// This code works in conjunction with fetch-data-slave-ROM-store-optimized.ino

// Hardware: an SIMB3 mainboard, powered with 18V, connected to the One-Wire Controller over 5V I2C

// Written 4 December, 2022 by Cameron Planck
// Updated 14 January, 2023 by Cameron Planck

#include <Wire.h>   
 
#define NUBC_5V_ENABLE          5
#define NUMDS28EA00s 80
                                                    
String readString;                                                          
byte command[2];   
byte topTempStringBuffer[160];
byte bottomTempStringBuffer[160];
byte airTempBuffer[2];
byte packedTempValues[NUMDS28EA00s*2*3/4];

void setup() {
  Wire.begin();                                                             
  Serial.begin(9600);
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  digitalWrite(NUBC_5V_ENABLE, HIGH); 
  delay(4000); // this delay is necessary in current form to allow the tempstring controller to spin up
}

void loop() {  
  
  Serial.println(F("\n\n================ BEGINNING I2C REQUEST ROUTINE ==================\n"));
  i2cRequestRoutine();
  delay(2000);
  
 }


void i2cRequestRoutine(){
    
     // first value is the command value, second is the packet index of data to send (can be 0-3)
    
    Serial.println("FETCHING TOP (UPPER) TEMPERATURE STRING VALUES FROM CONTROLLER");
    command[0] = 1;
    command[1] = 0;
    Wire.beginTransmission(11);                                                
    Wire.write(command,2);                                                    
    int error = Wire.endTransmission();  
    Serial.print("--> CONTROLLER RECEIEVED COMMAND WITH ERROR CODE : ");
    Serial.println(error);
    Serial.println("");
    delay(5000); 
 }



