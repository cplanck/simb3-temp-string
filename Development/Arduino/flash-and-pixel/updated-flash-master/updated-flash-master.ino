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
#define OW_CONTROLLER_I2C_ADDRESS 11
                                                    
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
    
    int error;
     // first value is the command value, second is the packet index of data to send (can be 0-3)
    
    Serial.println("FETCHING TOP (UPPER) TEMPERATURE STRING VALUES FROM CONTROLLER");
    command[0] = 1;
    command[1] = 0;
    Wire.beginTransmission(OW_CONTROLLER_I2C_ADDRESS);                                                
    Wire.write(command,2);                                                    
    error = Wire.endTransmission();  
    Serial.print("--> CONTROLLER RECEIEVED COMMAND WITH ERROR CODE : ");
    Serial.println(error);
    Serial.println("");
    delay(2000); 
    writeRequestToBuffer(1);

    delay(3000);

    Serial.println("FETCHING BOTTOM (LOWER) TEMPERATURE STRING VALUES FROM CONTROLLER");
    command[0] = 2;
    command[1] = 0;
    Wire.beginTransmission(OW_CONTROLLER_I2C_ADDRESS);                                                
    Wire.write(command,2);                                                    
    error = Wire.endTransmission();  
    Serial.print("--> CONTROLLER RECEIEVED COMMAND WITH ERROR CODE : ");
    Serial.println(error);
    Serial.println("");
    delay(2000); 
    writeRequestToBuffer(2);

    delay(3000);
    
    Serial.println("FETCHING AIR TEMPERATURE STRING VALUES FROM CONTROLLER");
    command[0] = 3;
    command[1] = 0; // packing index variable declaration not needed for air temperature, but we'll define it anyways   
    Wire.beginTransmission(OW_CONTROLLER_I2C_ADDRESS);                                       
    Wire.write(command,2);                                                    
    error = Wire.endTransmission();  
    Serial.print("--> CONTROLLER RECEIEVED COMMAND WITH ERROR CODE : ");
    Serial.println(error);
    
    delay(1000); 
    
    int k=0;
    Wire.requestFrom(OW_CONTROLLER_I2C_ADDRESS,2); 
    while (Wire.available()) {
        char c = Wire.read();
        airTempBuffer[k] = c;
        k++;
     }


//  printTemperatureRaw(1);
//  printTemperatureRaw(2);

  printTemperatureString(1);
  printTemperatureString(2);
  printAirTemperature();

  delay(1000);
  
 }


void writeRequestToBuffer(int tempstring){

    int k = 0;
    for(int i = 0; i < 5; i++){
     Wire.requestFrom(OW_CONTROLLER_I2C_ADDRESS,32); // request 32 bytes at a time
     while (Wire.available()) {
      char c = Wire.read();
      tempstring == 1 ?  topTempStringBuffer[k] = c : bottomTempStringBuffer[k] = c;
      k++;
    }
  }
}

 void printTemperatureRaw(int tempstring){

  int k = 1;
  Serial.println("\n");
  Serial.println(F("RAW TEMPERATURE STRING BINARY (DIRECT FROM I2C)"));

  tempstring == 1 ? Serial.println(F("TOP STRING:")) : Serial.println(F("BOTTOM STRING:"));
  
  if(tempstring == 1){
    for(int i = 0; i < 160; i=i+2){
      Serial.print(k);
      Serial.print(": ");
      Serial.print(topTempStringBuffer[i], BIN);
      Serial.print("  ");
      Serial.print(topTempStringBuffer[i+1], BIN);
      Serial.print(" ");
  
      if(i%6 == 0 & i != 0){
        Serial.println("");
        }
        k++;
      }
  }
  else if(tempstring == 2){
    for(int i = 0; i < 160; i=i+2){
      Serial.print(k);
      Serial.print(": ");
      Serial.print(bottomTempStringBuffer[i], BIN);
      Serial.print("  ");
      Serial.print(bottomTempStringBuffer[i+1], BIN);
      Serial.print(" ");
  
      if(i%6 == 0 & i != 0){
        Serial.println("");
        }
        k++;
      }
  }
}

void printTemperatureString(int tempstring){

   Serial.println("\n");
   Serial.println(F("CONVERTED TEMPERATURE STRING VALUES (DEG C)"));

   tempstring == 1 ? Serial.println(F("TOP STRING:")) : Serial.println(F("BOTTOM STRING:"));

   int k = 1;
   float ow_dtc_value;
   for(int i = 0; i<NUMDS28EA00s*2; i=i+2){
      if(tempstring == 1){
        ow_dtc_value = decodeTemperatureBytes(topTempStringBuffer[i], topTempStringBuffer[i+1]);
      }
      else if (tempstring == 2){
        ow_dtc_value = decodeTemperatureBytes(bottomTempStringBuffer[i], bottomTempStringBuffer[i+1]);
      }

      Serial.print(k);
      Serial.print(": ");
      Serial.print(ow_dtc_value);
      Serial.print(" ");
      if(i%8 == 0 & i!=0){
        Serial.println("");
        }
       k++;
    }
  }


void printAirTemperature(){

   Serial.println("\n");
   Serial.println("CONVERTED AIR TEMPERATURE VALUES (DEG C): ");
   float air_temp = decodeTemperatureBytes(airTempBuffer[0], airTempBuffer[1]);
   Serial.print(air_temp);
   Serial.print(" ");
}

float decodeTemperatureBytes(byte LSB, byte MSB){

  int bitArray[16];

  int k = 0;
  for(int i = 7; i>=0; i=i-1){
     int bits = bitRead(MSB,i);
     bitArray[k] = bits;
     k++;
    }
  
  for(int i = 7; i>=0; i=i-1){
     int bits = bitRead(LSB,i);
     bitArray[k] = bits;
     k++;
    }

   float temperature = (bitArray[5]*pow(2,6) + bitArray[6]*pow(2,5) + bitArray[7]*pow(2,4) + bitArray[8]*pow(2,3) + bitArray[9]*pow(2,2) + bitArray[10]*pow(2,1) + bitArray[11]*pow(2,0) + bitArray[12]*pow(2,-1) + bitArray[13]*pow(2,-2) + bitArray[14]*pow(2,-3) + bitArray[15]*pow(2,-4)); 
   return(temperature);
}

  byte* packTemperaturesForTransmission(int NumDS28AE00, byte *receiveBuffer, byte* packedBytes){

  // this function takes in the number of DS28EA00s to be packed and an array (receiveBuffer) of raw 16-bit temperature values

  // length of the buffer that holds the two bytes each chip returns for its temperature reading. Therefore this must be at least 2 * the number of chips. 
  #define receiveBufferLength NumDS28AE00 * 2

  // length of the packedBytes array, which will hold the packed 12-bit temperature readings on 8-bit byte boundaries. Because we go from 16-bit temp values to 12-bit, the length is 3/4 of receiveBuffer
  #define packedBytesLength receiveBufferLength*3/4

  // packed byte array. Generating this array is the output of this script.
//  byte packedBytes[packedBytesLength];

  // temporary variable for holding each row as it's appended to packedBytes
  byte packedRow[3];
  
  int count = 0; 
  byte LSB1;
  byte MSB1;
  byte LSB2; 
  byte MSB2; 
  byte maskedMSB1;
  byte maskedMSB2;

  for(int i = 0; i < receiveBufferLength; i=i+4){
    
    LSB1 = receiveBuffer[i];
    MSB1 = receiveBuffer[i+1];
    LSB2 = receiveBuffer[i+2];
    MSB2 = receiveBuffer[i+3];
  
    // add first measurement LSByte to row 1
    packedRow[0]=LSB1; 
  
    // isolate only last 4 bits of first measurement MSByte
    maskedMSB1 = MSB1 & 0b00001111; 
  
    // bit shift 4 to the right to give 4 trailing zeros
    maskedMSB1 = maskedMSB1 << 4;  
  
    // isolate last 4 bits of MSB2
    maskedMSB2 = MSB2 & 0b00001111; 
  
    // OR together to combine both into one byte
    packedRow[1] = maskedMSB1 | maskedMSB2; 
  
    //add second measurement LSB to row 3
    packedRow[2] = LSB2; 
  
    // finally, add packed row to packedBits array
    for(int j = 0; j < 3; j++){
      if(i == 0){
          packedBytes[i + j] = packedRow[j];
        }
       else{
          packedBytes[i-1 + j] = packedRow[j];
        }
    }
   count++;
  }

  return(packedBytes);
  }