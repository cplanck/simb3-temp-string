// Developement code for collecting values from the SIMB3 One-Wire Temperature String and sending them over Iridium. 
// It collects readings from the temp string, displays the raw values, displays the packed values, and sends them over Iridium
// hardware: 
//   -SIMB3 mainboard with Iridium modem attached. Board must have ~18V power (for 5V line).
//   -Iridium antenna attached.
//   -An SIMB3 One-Wire temperature string connected over I2C

// NOTE: NUBC_GPS_ENABLE must be pulled low for the modem to transmit. 
// written 4 December 2022 by Cameron Planck

#include <Arduino.h>
#include <Wire.h>
#include <IridiumSBD.h>

#define NUBC_IRIDIUM_ENABLE     13
#define NUBC_5V_ENABLE          5
#define NUBC_GPS_ENABLE         12
#define NUBC_WDT_RESET          A1

#define NumDS28AE00s 10
#define PackedBytesLength NumDS28AE00s * 2 * 3/4

IridiumSBD        iridium         (Serial1, NUBC_IRIDIUM_ENABLE);

int command;
int iridiumReturn;
byte rawTempValues[NumDS28AE00s*2];
byte packedTempValues[PackedBytesLength];

bool readingUpperString;

// ------------ SBD message -----------
typedef union {

  struct {
    byte     packedTempValues[120];
  } __attribute__((packed));
  uint8_t bytes[0];

} SBDMessage;

SBDMessage message;

// ------------------------------------

void setup() {
  
  Wire.begin();                                                             
  Serial.begin(9600);
//  configureIO();
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  digitalWrite(NUBC_5V_ENABLE, HIGH); 
  
}

void loop(){

  delay(5000);
  Serial.println("BEGINNING ROUTINE");
  clearMessage();

  readingUpperString = true;
  readUpperTempString();
  while(readingUpperString){}
  

  // add packed values to the message
  for(int i = 0; i < PackedBytesLength; i++){
    message.packedTempValues[i] = packedTempValues[i];
  }
  
  resetWatchdog();
  
//  Serial.println("TRANSMITTING ON IRIDIUM...");
//  iridiumOn();
//  iridiumReturn = sendIridium();
//  Serial.println(iridiumReturn);
//  Serial.println("TURNING IRIDIUM OFF...");
//  iridiumOff();
  
  delay(10000);
}


void readUpperTempString(){

    
    command = 1;
    Serial.println("");
    Serial.println("Reading Upper Temperature String");
    Wire.beginTransmission(1);                                                
    Wire.write(command);                                                    
    int error = Wire.endTransmission(); 
    Serial.println("WIRE ERROR");
    Serial.print(error);
    delay(4000); 
    Serial.print("Fetching data...");
    Wire.requestFrom(1,160); // request 160 bytes
    
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

//    packTemperaturesForTransmission(NumDS28AE00s, rawTempValues, packedTempValues);
//    Serial.println("");
//    Serial.println("Packed Temperature Values: ");
//    for(int i = 0; i < PackedBytesLength; i++){
//      Serial.print(packedTempValues[i]);
//      Serial.println(" ");
//    }
}


void configureIO(){
  
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  pinMode(NUBC_IRIDIUM_ENABLE, OUTPUT);
  pinMode(NUBC_GPS_ENABLE, OUTPUT);
  pinMode(NUBC_WDT_RESET, OUTPUT);
  digitalWrite(NUBC_IRIDIUM_ENABLE, LOW);
  digitalWrite(NUBC_5V_ENABLE, LOW);
  digitalWrite(NUBC_WDT_RESET, LOW);
  digitalWrite(NUBC_GPS_ENABLE, LOW);
  detachInterrupt(0);
  detachInterrupt(1);
  
}


void transmitIridium(){
  Serial.println(F("Transmitting on Iridium..."));
  iridiumOn();
  Serial.println(F("Iridium is on."));
  sendIridium();
  iridiumOff();  
}

void configureIridium()
{
  iridium.attachConsole(Serial);
  iridium.attachDiags(Serial);
  iridium.setPowerProfile(1);
  iridium.useMSSTMWorkaround(false);
}

void iridiumOn()
{
  digitalWrite(NUBC_IRIDIUM_ENABLE, HIGH);
  Serial.println("Turned on Iridium");
  Serial1.begin(19200);
  configureIridium();
  iridium.isAsleep();
  iridium.begin();
}

void iridiumOff()
{
  iridium.sleep();
  digitalWrite(NUBC_IRIDIUM_ENABLE, LOW);
}

int sendIridium(){
  iridium.sendSBDBinary(message.bytes, sizeof(message)); //actually transmit
}
//
//void decodeTemps(int numOfValues){
//  byte tempBytes[2];
//  int k = 1;
//  Serial.println("");
//  Serial.print("Results: ");
//  for(int i = 0; i < numOfValues; i=i+2){
//    tempBytes[0] = receiveBuffer[i];
//    tempBytes[1] = receiveBuffer[i+1];
//  
//    int32_t neg = 0x0;
//    if (tempBytes[1] & 0x80) // if the 
//      neg = 0xFFF80000;
//  
//    // shift and stack bits in MSbyte and LSbyte to form combined binary number
//    int32_t fpTemperature = 0;
//    fpTemperature = (((int16_t) tempBytes[1]) << 11)
//                    | (((int16_t) tempBytes[0]) << 3)
//                    | neg;
//  
//    // manually calculate temperature from raw
//    float convertedTemp = fpTemperature * 0.0078125f;
//    Serial.print(convertedTemp);
//    Serial.print(" ");
//
//    if(i%5 == 0){
//      Serial.println("");
//      }
//    k++;
//  }
//}

void clearMessage() {
  memset(message.bytes, 0, sizeof(message));
}


void resetWatchdog() {
  //Pets the WDT and keeps the program alive. If the WDT trips the chip will experience a hard reset.
  digitalWrite(NUBC_WDT_RESET, HIGH);
  delay(20);
  digitalWrite(NUBC_WDT_RESET, LOW);
}


// routine for generating packed temperature values for transmission over Iridium
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
