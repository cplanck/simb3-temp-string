// Same code as fetch-data-slave-ROM-store, but optimized to work with both TOP and BOTTOM temperature strings

// Hardware: a Feather M0 connected to the SIMB3 mainboard over 5V I2C. The one-wire wire devices should be connected over pins 12, 13.

// Written 14 January, 2023 by Cameron Planck

#include "Wire.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "Adafruit_NeoPixel.h"
#include "Express_M0_Flash.h"

bool masterIssuedCommand = false;

// state variable set by what master has requested
int state;

// index variable to indicate which packet to write
int packetIndex = 0;

// number of DS28EO00s to be read
# define chipNum 80

// define ROM array, which needs to be at minimum 8*(number of DS28EA00s)
byte ROMarray[8 * chipNum];

// define tempArray which needs to be at least 2*(number of DS28EA00s)
byte tempArray[2 * chipNum];

// define airTempArray which holds the two bytes from the DS18B20 air temperature sensor
byte airTempArray[2];

byte present = 0;

// 32-byte short buffer for i2c transmission
byte tempPacket[32];

#define ROM_SEARCH_ENABLE 5

Express_M0_Flash onBoardFlash;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);

// bottom string one-wire instance
OneWire  bs(11);  // on pin 12 (a 4.7K (ish) resistor is necessary, it wont work without it)

// top string one-wire instance
OneWire  ts(12);  // on pin 12 (a 4.7K (ish) resistor is necessary, it wont work without it)

// air-temp one-wire instance
OneWire  at(13);  //  on pin 13 (a 4.7K (ish) resistor is necessary, it wont work without it)

// Pass our oneWire reference to Dallas Temperature library
DallasTemperature top_string(&ts);
DallasTemperature bottom_string(&bs);
DallasTemperature air_temp(&at);

void setup() {
  Serial.begin(9600);
  Wire.begin(11);
  pinMode(ROM_SEARCH_ENABLE, INPUT_PULLUP); 
  digitalWrite(ROM_SEARCH_ENABLE, LOW);
  onBoardFlash.initialize();
  pixels.begin();
  pixels.setBrightness(0);

  // runs when master sends data
  Wire.onReceive(receiveEvent);

  // runs when master asks for data
  Wire.onRequest(requestEvent);

  // start up one-wire devices
  top_string.begin();
  bottom_string.begin();
  air_temp.begin();
}


void receiveEvent(int numBytes) {
  if (Wire.available()){
    state = Wire.read(); // first byte in "command"
    packetIndex = Wire.read(); // second byte in "command"
  }
  masterIssuedCommand = true;
}

void requestEvent() {
  if(state != 3){
    packTempPacket(packetIndex);
    Wire.write(tempPacket, 32);
    packetIndex++;
  }
  else{
     Wire.write(airTempArray, 2);
     packetIndex++; // packet++ still necessary to prevent readAirTemp() from being called twice
  }

}

void loop() {
  if(masterIssuedCommand && packetIndex == 0){
    
    if (state == 1){
      readTemperatureString(1, top_string, ts);
      masterIssuedCommand = false;
    }
    
    else if (state == 2 && packetIndex == 0){
      readTemperatureString(2, bottom_string, bs);
      masterIssuedCommand = false;
    }

    else if (state == 3 && packetIndex == 0){
      readAirTemp();
      masterIssuedCommand = false;
    }
  }
}


// function to grab a subset of data from the temperature string buffer.
void packTempPacket(int ind) {
  for (int i = 0; i < 32; i++) {
    tempPacket[i] = tempArray[ind * 32 + i];
  }
}


void readTemperatureString(int tempstring, DallasTemperature dt, OneWire ow) {

  // tempstring = 1: top string
  // tempstring = 2: bottom string

  if(tempstring == 1){
      Serial.println("\n===================== READING TOP TEMPERATURE STRING ===================== ");
  }else{
      Serial.println("\n=================== READING BOTTOM TEMPERATURE STRING ==================== ");
  }
  

  Serial.println("Initialize One-Wire....");
  Serial.print("Number of one-wire devices detected: ");
  Serial.println(dt.getDeviceCount());

  // if pin 5 is HIGH, execute the chip discovery process and store ROMs to onboard Flash memory
  if (digitalRead(ROM_SEARCH_ENABLE) == HIGH) {
    pixel(true, "PURPLE");
    Serial.println("ROM SEARCH ENABLED. DISCOVERING ROMS FOR STRINGS...");
    writeROMsToFlash();
    pixel(false, "PURPLE");
  }
  
  // Retrieve ROMs from onboard Flash memory
    Serial.println("FETCHING TEMPERATURE ROMS FROM FLASH...");
    if(tempstring == 1){
          pixel(true, "RED");
          onBoardFlash.readbytes("TOPSTRING_ROMS.csv", ROMarray, 640);
          pixel(false, "RED");
      }else{
          pixel(true, "GREEN");
          onBoardFlash.readbytes("BOTTOMSTRING_ROMS.csv", ROMarray, 640);
          pixel(false, "GREEN");
    }

  // ask all chips to perform temperature conversion
  dt.requestTemperatures();

  present = ow.reset();

  // loop over ROMarray, select each chip, ask for temperature, and write to tempArray
  Serial.println("Requesting temperatures from DS28EA00s...");
  for (int i = 0; i < chipNum; i++) {
    ow.reset();
    chipSelect(i, ow);
  }

  printTemperatureString();

}

void writeROMsToFlash(){

   ts.reset();
   bs.reset();  
   
   onBoardFlash.format();

   top_string.chainOn();
   for (int i = 0; i < chipNum; i++) {
        chipDiscover(i, top_string, ts);
   };
   onBoardFlash.writebytes("TOPSTRING_ROMS.csv", ROMarray, 640);
   Serial.println("Sucessfully wrote TOPSTRING_ROMS.csv\n");

   bottom_string.chainOn();
   for (int i = 0; i < chipNum; i++) {
        chipDiscover(i, bottom_string, bs);
   };
   onBoardFlash.writebytes("BOTTOMSTRING_ROMS.csv", ROMarray, 640);
   Serial.println("Sucessfully wrote BOTTOMSTRING_ROMS.csv\n");
      
}


void chipDiscover(int k, DallasTemperature dt, OneWire ow) {

  byte addr[8];
  
  // reset 1-wire bus
  present = ow.reset();

  // execute conditional read ROM command
  ow.write(0x0F);
  for (int i = 0; i < 8; i++) {
    addr[i] = ow.read();
    ROMarray[k * 8 + i] = addr[i];
  }

  // issue chain done command to chip
  dt.chainDone();

}


void chipSelect(int k, OneWire ow) {

  byte addr[8];
  ow.write(0x55); // apply Match ROM command and select chip by writing ROM
  for (int i = 0; i < 8; i++) {
    addr[i] = ROMarray[k * 8 + i];
    ow.write(addr[i]);
  }

  ow.write(0xBE); // read scratchpad command
  byte scratchpad[2];
  for (int i = 0; i < 2; i++) { // step through each byte, write it to ROMarray
    scratchpad[i] = ow.read();
  }

  // add MSB and LSB to tempArray
  for (int i = 0; i < 2; i++) {
    tempArray[k * 2 + i] = scratchpad[i];
  }
}


void readAirTemp() {

  Serial.println("\n================= READING DS18B20 AIR TEMPERATURE SENSOR ================= ");
  Serial.println("Fetching air temperature...");
  air_temp.requestTemperatures();
  at.reset(); // reset one-wire bus


  pixel(true, "BLUE");
  Serial.println("Getting air temperature bytes...");
  at.write(0xCC); // skip ROM
  at.write(0xBE); // read scratchpad command
  byte scratchpad[8];
  for (int i = 0; i < 2; i++) { // step first two bytes, write them to scratchpad
    scratchpad[i] = at.read();
  }

  for (int i = 0; i < 2; i++) {
    airTempArray[i] = scratchpad[i];
  }

  printAirTemperature();
  pixel(false, "BLUE");
}

void printAirTemperature() {

  Serial.println("CONVERTED AIR TEMPERATURE VALUES (DEG C): ");
  float air_temp = decodeTemperatureBytes(airTempArray[0], airTempArray[1]);
  Serial.print(air_temp);
  Serial.println(" ");
}

void printTemperatureString() {

  Serial.println(F("CONVERTED TEMPERATURE STRING VALUES (DEG C): "));
  int k = 1;
  for (int i = 0; i < chipNum * 2; i = i + 2) {
    float ow_dtc_value = decodeTemperatureBytes(tempArray[i], tempArray[i + 1]);
    Serial.print(k);
    Serial.print(": ");
    Serial.print(ow_dtc_value);
    Serial.print(" ");
    if (i % 8 == 0 & i != 0) {
      Serial.println("");
    }
    k++;
  }
  Serial.println("");
}

float decodeTemperatureBytes(byte LSB, byte MSB) {

  int bitArray[16];

  int k = 0;
  for (int i = 7; i >= 0; i = i - 1) {
    int bits = bitRead(MSB, i);
    bitArray[k] = bits;
    k++;
  }

  for (int i = 7; i >= 0; i = i - 1) {
    int bits = bitRead(LSB, i);
    bitArray[k] = bits;
    k++;
  }

  float temperature = (bitArray[5] * pow(2, 6) + bitArray[6] * pow(2, 5) + bitArray[7] * pow(2, 4) + bitArray[8] * pow(2, 3) + bitArray[9] * pow(2, 2) + bitArray[10] * pow(2, 1) + bitArray[11] * pow(2, 0) + bitArray[12] * pow(2, -1) + bitArray[13] * pow(2, -2) + bitArray[14] * pow(2, -3) + bitArray[15] * pow(2, -4));
  return (temperature);
}


void pixel(bool on, char* color){

  if(on){
      pixels.setBrightness(50);
  }
  else{
      pixels.setBrightness(0);
  }

  if(color == "GREEN"){
      pixels.setPixelColor(0, pixels.Color(45,255,0));
  }
  else if(color == "BLUE"){
          pixels.setPixelColor(0, pixels.Color(0,255,255));
  }
  else if(color == "PURPLE"){
          pixels.setPixelColor(0, pixels.Color(121,0,255));
  }
  else if(color == "RED"){
          pixels.setPixelColor(0, pixels.Color(255,0,0));
  }

  pixels.show();
}