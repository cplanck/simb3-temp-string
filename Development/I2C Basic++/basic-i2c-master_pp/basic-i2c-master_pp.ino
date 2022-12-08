// Basic I2C controller (master) code for iterative communication beteween the SIMB3 mainboard and the One-wire Temperature String Controller
// written 4 December, 2022 by Cameron Planck

#include <Wire.h>   
 
#define NUBC_5V_ENABLE          5
#define NUMDS28EA00s 80
                                                    
String readString;                                                          
byte command[2];   
byte tempStringBuffer[160];
byte airTempBuffer[2];
byte packedTempValues[NUMDS28EA00s*2*3/4];

void setup() {
  Wire.begin();                                                             
  Serial.begin(9600);
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  digitalWrite(NUBC_5V_ENABLE, HIGH); 
}

void loop() {

  delay(4000); // this delay is necessary in current form to allow the tempstring controller to spin up

  // get temperature string values from temperature string controller
  i2cRequestRoutine();
  
  delay(2000);
  
 }


void i2cRequestRoutine(){

    // iteratively request values over i2c and copy to buffer
    
     // first value is the command value, second is the index of data to send (can be 0-3)
    
//    // REQUEST TEMPERATURE STRING:
    command[0] = 1;
    command[1] = 0;
    Serial.println(F("\n\n==================== BEGINNING ROUTINE ======================\n"));
    Serial.println("MASTER ASKING CONTROLLER TO FETCH TEMPERATURE STRING VALUES...");
    Wire.beginTransmission(1);                                                
    Wire.write(command,2);                                                    
    int error = Wire.endTransmission();  
    Serial.print("CONTROLLER RECEIEVED COMMAND WITH ERROR CODE : ");
    Serial.println(error);
    
    delay(2000); 

    Serial.print(F("MASTER ITERATIVELY REQUESTING TEMPERATURE STRING BYTES FROM CONTROLLER..."));

    int k = 0;
    for(int i = 0; i < 5; i++){
       Wire.requestFrom(1,32); // request 32 bytes at a time
       while (Wire.available()) {
        char c = Wire.read();
        tempStringBuffer[k] = c;
        k++;
      }
    }

    // REQUEST AIR TEMPERATURE:
    Serial.println("\n\nMASTER ASKING CONTROLLER TO FETCH AIR TEMPERATURE STRING VALUES...");
    command[0] = 2;
    command[1] = 0; // packing index variable declaration not needed for air temperature, but we'll define it anyways   
    Wire.beginTransmission(1);                                       
    Wire.write(command,2);                                                    
    error = Wire.endTransmission();  
    Serial.print("CONTROLLER RECEIEVED COMMAND WITH ERROR CODE : ");
    Serial.println(error);
    
    delay(2000); 

    Serial.print("MASTER REQUESTING AIR TEMPERATURE STRING BYTES FROM CONTROLLER...");

    k=0;
    Wire.requestFrom(1,2); // request 32 bytes at a time
    while (Wire.available()) {
        Serial.print("WIRE AVAILABLE CALLED");
        char c = Wire.read();
        airTempBuffer[k] = c;
        k++;
     }


  delay(1000);
  //FOR SOME REASON PACKING THE TEMPERATURE VALUES MESSES WITH THE CONVERTED OUTPUT
  
  packTemperaturesForTransmission(NUMDS28EA00s, tempStringBuffer, packedTempValues);

  
  printTemperatureRaw();
  printTemperatureString();
  printAirTemperature();
  printPackedBinary();
  
 }


 void printTemperatureRaw(){

  int k = 1;
  Serial.println("\n");
  Serial.println(F("RAW TEMPERATURE STRING BINARY (DIRECT FROM I2C):"));
  
  for(int i = 0; i < 160; i=i+2){
    Serial.print(k);
    Serial.print(": ");
    Serial.print(tempStringBuffer[i], BIN);
    Serial.print("  ");
    Serial.print(tempStringBuffer[i+1], BIN);
    Serial.print(" ");

    if(i%6 == 0 & i != 0){
      Serial.println("");
      }
      k++;
    }
}

void printTemperatureString(){

  Serial.println("\n");
   Serial.println(F("CONVERTED TEMPERATURE STRING VALUES (DEG C): "));
   int k = 1;
   for(int i = 0; i<NUMDS28EA00s*2; i=i+2){
      float ow_dtc_value = decodeTemperatureBytes(tempStringBuffer[i], tempStringBuffer[i+1]);
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


void packTemperaturesForTransmission(int NumDS28AE00, byte receiveBuffer[160], byte* packedBytes){

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

  int packedCount = 0;
  for(int i = 0; i < receiveBufferLength-3; i=i+4){
    
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
  
    // finally, add packed row to packedBytes array
    for(int j = 0; j < 3; j++){
          packedBytes[packedCount] = packedRow[j];
          packedCount++;
        }
    }
   count++;
}

void printPackedBinary(){

  Serial.println(F("\n"));
   Serial.println(F("PACKED TEMPERATURE STRING BYTES: "));
   int k = 1;
   for(int i = 0; i<NUMDS28EA00s*2*3/4; i++){
      Serial.print(k);
      Serial.print(": ");
      Serial.print(packedTempValues[i], BIN);
      Serial.print(" ");
      if(i%8 == 0 & i!=0){
        Serial.println("");
        }
       k++;
    }
  }
