// Developement code for testing bare-bones Iridium communication using the SIMB3 mainboard. The code simply writes a set of test values (testValue) to the message and transmits over Iridium. 
// hardware: an SIMB3 mainboard with Iridium modem attached. Board must have ~18V power (for 5V line) and an antenna attached.
// NOTE: NUBC_GPS_ENABLE must be pulled low for the modem to transmit. 
// written 4 December 2022 by Cameron Planck

#include <Arduino.h>
#include <Wire.h>
#include <IridiumSBD.h>

#define NUBC_IRIDIUM_ENABLE     13
#define NUBC_5V_ENABLE          5
#define NUBC_GPS_ENABLE         12
#define NUBC_WDT_RESET          A1

IridiumSBD        iridium         (Serial1, NUBC_IRIDIUM_ENABLE);

int command;
int iridiumReturn;
byte receiveBuffer[160];

//byte testValue = 0x7B;
byte testValue[4] = {0x7B, 0x3C, 0xF, 0x11};


// ------------ SBD message -----------
typedef union {

  struct {
//    int32_t     timestamp;
    byte     testValue[4];
  } __attribute__((packed));
  uint8_t bytes[0];

} SBDMessage;

SBDMessage message;

// ------------------------------------

void setup() {
  
  Wire.begin();                                                             
  Serial.begin(9600);
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  digitalWrite(NUBC_5V_ENABLE, HIGH); 
  configureIO();
  
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

void loop(){

  clearMessage();

  for(int i = 0; i < 4; i++){
    message.testValue[i] = testValue[i];
  }
  
  
  digitalWrite(NUBC_5V_ENABLE, HIGH);
  resetWatchdog();
  Serial.println("TRANSMITTING ON IRIDIUM...");
  iridiumOn();
  iridiumReturn = sendIridium();
  Serial.println(iridiumReturn);
  Serial.println("TURNING IRIDIUM OFF...");
  iridiumOff();
  delay(10000);
}


void readUpperTempString(){
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
    k++;
    decodeTemps(160);
    }
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

void clearMessage() {
  memset(message.bytes, 0, sizeof(message));
}


void resetWatchdog() {
  //Pets the WDT and keeps the program alive. If the WDT trips the chip will experience a hard reset.
  digitalWrite(NUBC_WDT_RESET, HIGH);
  delay(20);
  digitalWrite(NUBC_WDT_RESET, LOW);
}
