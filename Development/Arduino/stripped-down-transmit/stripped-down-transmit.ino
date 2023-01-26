////////////////////////////////////////////////////////////
// Development SIMB3 code for validating SIMB3 one-wire controller communication and transmission (and decoding)
// Written 19 January 2023 by Cameron Planck

#define DEBUG 1
#include <Arduino.h>
#include <Wire.h>
#include <RTCZero.h>
#include <LineBuffer.h>
#include "wiring_private.h" // pinPeripheral() function
#include <SPI.h>

#include <IridiumSBD.h>
#include <GPS.h>
#include <LTC2945.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Maxbotix.h>
#include <AirmarSS510.h>
#include <SFE_LSM9DS0_TAV.h>
#include <BruncinDTC.h>
#include <DS2482.h>
#include <Adafruit_ADS1015.h>
#include <SD.h>
#include "SIMB3_Onewire_Controller.h"


// Version ----------------------------------------------------------

#define PROGRAM_VERSION         0x00

#define SBD_RECORD_ID           0xA0
#define SBD_RECORD_VERSION      0x00
#define SBD_RECORD_HEADER       (SBD_RECORD_ID | SBD_RECORD_VERSION)

// Program Options --------------------------------------------------

#define IRIDIUM_ATTEMPTS         10
#define IRIDIUM_RETRY_DELAY      10000    //10 seconds
#define GPS_TIMEOUT              6    //60 seconds

// Hardware

#define SIM3_ONEWIRE_TEMPSTRING true

// Pin mappings -----------------------------------------------------

#define NUBC_MAXBOTIX_ENABLE    A0
#define NUBC_ENVELOPE_ENABLE    A1
#define NUBC_GPS_ENABLE         12
#define NUBC_IRIDIUM_ENABLE     13
#define NUBC_5V_ENABLE          5
#define NUBC_12V_ENABLE         6

#define NUBC_LED_G              A3
#define NUBC_CHIP_SELECT        A4

#define NUBC_WDT_RESET          A1

// I2C Addresses ----------------------------------------------------

#define NUBC_GYRO_ADDR          0x6A
#define NUBC_MAG_ADDR           0x1E
#define NUBC_POWERMON_ADDR      0x6F
#define NUBC_BMP280             0x77
#define CIBC_ADS                0x48

#define NUBC_POWERMON_RESISTOR  0.02
#define TAC_BYTES               120

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

Uart          Serial2         (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

LTC2945           powermon        (NUBC_POWERMON_ADDR, NUBC_POWERMON_RESISTOR);
DS2482            oneWireA        (0);
DS2482            oneWireB        (1);

IridiumSBD        iridium         (Serial1, NUBC_IRIDIUM_ENABLE);
Adafruit_GPS      GPS             (&Serial1);
RTCZero           rtc;    // Create RTC object
BruncinDTC        dtc             (&Serial1);
AirmarSS510       airmar          (Serial2);
Maxbotix          maxbotix        (Serial2);
Adafruit_BME280   bme; // I2C

SIMB3_Onewire_Controller SIMB3_ONEWIRE_CONTROLLER;


boolean           errorState;
//boolean           transmitSuccessful = true;

uint32_t          startTime;
uint32_t          prevTime;
float             accPower;

int               iridiumSignal;
uint8_t           iridiumCount;
int               iridiumError;
int               NextAlarmHour; // Variable to hold next alarm time in hours

/* Change these values to set the current initial time */
const byte hours = 13;
const byte minutes = 49;
const byte seconds = 33;
/* Change these values to set the current initial date */
const byte DAY = 15;
const byte MONTH = 11;
const byte YEAR = 18;

//set to 1 for hourly transmits, 4 for every 4 hours
int transmissionInterval = 4;

int iSQ;

char fileName[13];

int sleepCycleCount;
uint8_t runCounter;
bool sdPresent; 

// SBD message format -----------------------------------------------

typedef union {

  struct {
    byte airTemperatureMessage[2];
    byte topTempStringMessage[120];
    byte bottomTempStringMessage[120];

  } __attribute__((packed));

  uint8_t bytes[0];

} SBDMessage;

SBDMessage message;


void clearMessage() {
  memset(message.bytes, 0, sizeof(message));
}


/////////////////////////////////////////////////////////////////////

void setup()
{

  Serial.begin(115200);
  Serial.println("\r\nCryosphere Innovation CryoBoard");
  Wire.begin(); // Join the I2C bus as master

  configureIO();
  resetWatchdog(); //Pet the watchdog

  digitalWrite(NUBC_LED_G, HIGH);
}

void loop()
{
  resetWatchdog(); //Pet the watchdog
  startTime = prevTime = millis();
  
  Serial.println(F("Begin loop()..."));

  clearMessage();

  powermon.wakeup();

if(SIM3_ONEWIRE_TEMPSTRING)
{
  readOnewireController();
}
else{
  // Read Bruncin DTC...
}

// -------Disable Iridium transmission for testing--------
  Serial.println(F("Transmitting on Iridium..."));
  iridiumOn();
  Serial.println(F("Iridium is on."));
  resetWatchdog(); //Pet the watchdog
  sendIridium();
  iridiumOff();
  Serial.println(F("Done transmitting..."));
  Serial.print(F("Iridium status: "));
  Serial.println(iridiumError);
  if (iridiumError == 0) {
    digitalWrite(NUBC_LED_G, LOW);
  }
// -------Disable Iridium transmission for testing--------


  digitalWrite(NUBC_12V_ENABLE, HIGH);
  digitalWrite(NUBC_5V_ENABLE, LOW);

  // powermon.shutdown();
  delay(5000);
};


void configureIO()
{
  pinMode(NUBC_MAXBOTIX_ENABLE, OUTPUT);
  //pinMode(NUBC_ENVELOPE_ENABLE, OUTPUT); //Not used
  pinMode(NUBC_GPS_ENABLE, OUTPUT);
  pinMode(NUBC_IRIDIUM_ENABLE, OUTPUT);
  pinMode(NUBC_5V_ENABLE, OUTPUT);
  pinMode(NUBC_12V_ENABLE, OUTPUT);
  pinMode(NUBC_LED_G, OUTPUT);
  pinMode(NUBC_WDT_RESET, OUTPUT);

  digitalWrite(NUBC_MAXBOTIX_ENABLE, LOW);
  digitalWrite(NUBC_GPS_ENABLE, LOW);
  digitalWrite(NUBC_IRIDIUM_ENABLE, LOW);
  digitalWrite(NUBC_5V_ENABLE, LOW);
  digitalWrite(NUBC_12V_ENABLE, HIGH);
  digitalWrite(NUBC_LED_G, LOW);
  digitalWrite(NUBC_WDT_RESET, LOW);

  detachInterrupt(0);
  detachInterrupt(1);

  Serial.print(F("32"));
  powermon.wakeup();
  Serial.print(F("32"));
  powermon.write(LTC2945_CONTROL_REG, LTC2945_SENSE_MONITOR);
}

void configureIridium()
{
  Serial.println(F("Configuring the iridium"));
  iridium.attachConsole(Serial);
  iridium.attachDiags(Serial);
  //iridium.begin();
  Serial.println(F("after iridium begin"));
  iridium.setPowerProfile(1);
  Serial.println(F("after set power profile"));
  iridium.useMSSTMWorkaround(false);
}

void iridiumOn()
{
  digitalWrite(NUBC_IRIDIUM_ENABLE, HIGH);
  Serial.println(F("Turned on the iridium"));
  Serial1.begin(19200);
  configureIridium();
  Serial.println(F("configured the iridium"));
  //  delay(100);
  iridium.isAsleep();
  iridium.begin();
}

void iridiumOff()
{
  iridium.sleep();
  digitalWrite(NUBC_IRIDIUM_ENABLE, LOW);
}

void sendIridium()
{
  iridiumSignal = -1;
  iridiumCount  = 0;
  iridiumError  = -1;
  iridium.getSignalQuality(iridiumSignal);

  
  iridiumError = iridium.sendSBDBinary(message.bytes, sizeof(message)); //actually transmit
 
}


void resetWatchdog() {
  //Pets the WDT and keeps the program alive. If the WDT trips the chip will experience a hard reset.
  digitalWrite(NUBC_WDT_RESET, HIGH);
  delay(20);
  digitalWrite(NUBC_WDT_RESET, LOW);
}

void readOnewireController(){

  byte topTempStringBuffer[160];
  byte topTempStringPackedBuffer[120];
  byte bottomTempStringBuffer[160];
  byte bottomTempStringPackedBuffer[120];
  byte airTemperatureBuffer[2];

  Serial.println(F("Resetting One-Wire controller..."));
  SIMB3_ONEWIRE_CONTROLLER.reset();
  Serial.println(F("Controller reset."));

  Serial.println(F("Reading top temperature string..."));
  SIMB3_ONEWIRE_CONTROLLER.readTopString();

  delay(2000);
  
  SIMB3_ONEWIRE_CONTROLLER.requestTemperatureStringBytes(1, topTempStringBuffer);
  SIMB3_ONEWIRE_CONTROLLER.packTemperaturesForTransmission(80, topTempStringBuffer, topTempStringPackedBuffer);
  SIMB3_ONEWIRE_CONTROLLER.printTemperatureString(1, topTempStringBuffer);

  for(int i = 0; i < 120; i++){
    message.topTempStringMessage[i] = topTempStringPackedBuffer[i];
  }

  Serial.println(F("Reading bottom temperature string..."));
  SIMB3_ONEWIRE_CONTROLLER.readBottomString();
  delay(2000);
  
  SIMB3_ONEWIRE_CONTROLLER.requestTemperatureStringBytes(2, bottomTempStringBuffer);
  SIMB3_ONEWIRE_CONTROLLER.packTemperaturesForTransmission(80, bottomTempStringBuffer, bottomTempStringPackedBuffer);
  SIMB3_ONEWIRE_CONTROLLER.printTemperatureString(2, bottomTempStringBuffer);

  for(int i = 0; i < 120; i++){
    message.bottomTempStringMessage[i] = bottomTempStringPackedBuffer[i];
  }

  Serial.println(F("Reading air temperature..."));
  SIMB3_ONEWIRE_CONTROLLER.readAirTemperature();
  delay(2000);

  SIMB3_ONEWIRE_CONTROLLER.requestAirTempBytes(airTemperatureBuffer);
  SIMB3_ONEWIRE_CONTROLLER.printAirTemperature(airTemperatureBuffer);
  
  for(int i = 0; i < 2; i++){
    message.airTemperatureMessage[i] = airTemperatureBuffer[i];
  }
  
}
