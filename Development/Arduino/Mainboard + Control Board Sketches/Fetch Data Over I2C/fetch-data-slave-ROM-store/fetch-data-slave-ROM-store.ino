// Development code to communicate and send data from the SIMB3 One-Wire Controller

// Development code for the SIMB3 One-Wire Controller.

// Code listens for I2C commands from the SIMB3 mainboard which instructs it what data to fetch from the attached one-wire devices.
// The controller then requests the data from the one-wire devices and writes it back to the master.
// This code works in conjunction with buffer-fix-master.ino

// Hardware: a Feather M0 connected to the SIMB3 mainboard over 5V I2C. The one-wire wire devices should be connected over pins 12, 13.

// Written 4 December, 2022 by Cameron Planck

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Express_Flash.h>

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

Adafruit_Express_Flash onBoardFlash;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);


#define ROM_SEARCH_ENABLE 5

void setup() {
  Serial.begin(9600);

  // set ROM search pin (pin 5)
  pinMode(ROM_SEARCH_ENABLE, INPUT); 

  // intitialize flash storage
  onBoardFlash.initialize();

  // enable pixel RBG
  pixels.begin();
  pixels.setBrightness(0);


  // initialize I2C with address 1
  Wire.begin(1);

  // runs when master sends data
  Wire.onReceive(receiveEvent);

  // runs when master asks for data
  Wire.onRequest(requestEvent);

  // start up one-wire devices
  top_string.begin();
  bottom_string.begin();
  air_temp.begin();
 

}

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

// called when master sends data
void receiveEvent(int numBytes) {
  while (Wire.available()) {
    state = Wire.read(); // first byte in "command"
    packetIndex = Wire.read(); // second byte in "command"
  }
}

// called when master asks for data
void requestEvent() {
  if (state == 1 && packetIndex == 0) {
    readTopString();
    packTempPacket(packetIndex);
    Wire.write(tempPacket, 32);
    packetIndex++;
  }

  else if (state == 1) {
    packTempPacket(packetIndex);
    Wire.write(tempPacket, 32);
    packetIndex++;
  }

  else if (state == 2 && packetIndex == 0) {
    readBottomString();
    packTempPacket(packetIndex);
    Wire.write(tempPacket, 32);
    packetIndex++;
  }

  else if (state == 2) {
    packTempPacket(packetIndex);
    Wire.write(tempPacket, 32);
    packetIndex++;
  }
  else if (state == 3) {
    readAirTemp();
    Wire.write(airTempArray, 2);
  }
}


// function to grab a subset of data from the temperature string buffer.
void packTempPacket(int ind) {
  for (int i = 0; i < 32; i++) {
    tempPacket[i] = tempArray[ind * 32 + i];
  }
}


void readTopString() {

  Serial.println("===================== READING TOP TEMPERATURE STRING ===================== ");
  Serial.println("Initialize One-Wire....");
  Serial.print("Number of one-wire devices detected: ");
  Serial.println(top_string.getDeviceCount());

  // ask all chips to perform temperature conversion
  top_string.requestTemperatures();

  present = ts.reset();
  Serial.print("Present?: ");
  Serial.println(present);

  // turn chain mode on
  top_string.chainOn();

  // run chip discover process to generate ROMarray
  // if pin 2 is pulled high, execute the chip discovery process and store to onboard Flash memory
  if (digitalRead(ROM_SEARCH_ENABLE) == HIGH) {
    pixels.setBrightness(50);
    pixels.setPixelColor(0, pixels.Color(121,0,255));
    pixels.show();
    
    Serial.println("ROM SEARCH ENABLED");
    Serial.println("Beginning chip discovery...");
    for (int i = 0; i < chipNum; i++) {
      chipDiscover(i);
    };
    onBoardFlash.format();
    onBoardFlash.writebytes("TOPSTRING_ROMS.csv", ROMarray, 640);
    pixels.setBrightness(0);
    pixels.show();
  }
  
  // if pin 5 is NOT pulled high, retrieve ROMs from onboard Flash memory
  else {
    pixels.setBrightness(50);
    pixels.setPixelColor(0, pixels.Color(0,255,14));
    pixels.show();
    Serial.println("ROM SEARCH DISABLED");
    Serial.println("Getting temperature bytes...");
    onBoardFlash.readbytes("TOPSTRING_ROMS.csv", ROMarray, 640);
    for (int i = 0; i < chipNum; i++) {
      ts.reset();
      chipSelect(i);
    }
    pixels.setBrightness(0);
    pixels.show();
  }


  // loop over ROMarray, select each chip, ask for temperature, and write to tempArray
  Serial.println("Getting temperature bytes...");
  for (int i = 0; i < chipNum; i++) {
    ts.reset();
    chipSelect(i);
  }

  printTemperatureString();

}


void readBottomString() {

  Serial.println("===================== READING BOTTOM TEMPERATURE STRING ===================== ");
  Serial.println("Initialize One-Wire....");
  Serial.print("Number of one-wire devices detected: ");
  Serial.println(bottom_string.getDeviceCount());

  // ask all chips to perform temperature conversion
  bottom_string.requestTemperatures();

  present = bs.reset();
  Serial.print("Present?: ");
  Serial.println(present);

  // turn chain mode on
  bottom_string.chainOn();

  // run chip discover process to generate ROMarray
  // if pin 2 is pulled high, execute the chip discovery process and store to onboard Flash memory
  if (digitalRead(ROM_SEARCH_ENABLE) == HIGH) {
    pixels.setBrightness(50);
    pixels.setPixelColor(0, pixels.Color(121,0,255));
    pixels.show();
    
    Serial.println("ROM SEARCH ENABLED");
    Serial.println("Beginning chip discovery...");
    for (int i = 0; i < chipNum; i++) {
      chipDiscover(i);
    };
    onBoardFlash.format();
    onBoardFlash.writebytes("BOTTOMSTRING_ROMS.csv", ROMarray, 640);
    pixels.setBrightness(0);
    pixels.show();
  }
  
  // if pin 5 is NOT pulled high, retrieve ROMs from onboard Flash memory
  else {
    pixels.setBrightness(50);
    pixels.setPixelColor(0, pixels.Color(0,255,14));
    pixels.show();
    Serial.println("ROM SEARCH DISABLED");
    Serial.println("Getting temperature bytes...");
    onBoardFlash.readbytes("TOPSTRING_ROMS.csv", ROMarray, 640);
    for (int i = 0; i < chipNum; i++) {
      bs.reset();
      chipSelect(i);
    }
    pixels.setBrightness(0);
    pixels.show();
  }


  // loop over ROMarray, select each chip, ask for temperature, and write to tempArray
  Serial.println("Getting temperature bytes...");
  for (int i = 0; i < chipNum; i++) {
    bs.reset();
    chipSelect(i);
  }

  printTemperatureString();

}





void chipDiscover(int k) {

  // note: sensors.chainOn() must be called before the chipDiscover routine
  byte addr[8];
  byte i;

  // reset 1-wire bus
  present = ts.reset();

  // execute conditional read ROM command and print response (64-bit registration number)
  //    Serial.println("Applying Conditional Read ROM Command...");
  ts.write(0x0F);
  //    Serial.print("Chip Address: ");
  for (i = 0; i < 8; i++) {
    addr[i] = ts.read();
    //      Serial.print(addr[i], HEX);
    //      Serial.print(" ");
    ROMarray[k * 8 + i] = addr[i];
  }

  //    Serial.println("");

  // turn chain mode off
  top_string.chainDone();

}

void chipSelect(int k) {

  // apply Match ROM command and select chip by writing ROM
  //    Serial.println("Selecting chip by ROM...");
  byte addr[8];
  ts.write(0x55);
  for (int i = 0; i < 8; i++) {
    addr[i] = ROMarray[k * 8 + i];
    ts.write(addr[i]);
    //      Serial.print(addr[i], HEX);
    //      Serial.print(" ");
  }

  //    Serial.println("");

  //    Serial.println("Reading Scratchpad...");
  ts.write(0xBE); // read scratchpad command
  byte scratchpad[2];
  //    Serial.print("Scratchpad Bytes: ");
  for (int i = 0; i < 2; i++) { // step through each byte, write it to ROMarray
    scratchpad[i] = ts.read();
    //      Serial.print(scratchpad[i], BIN);
    //      Serial.print(" ");
  }
  //    Serial.println("");

  // add MSB and LSB to tempArray
  for (int i = 0; i < 2; i++) {
    tempArray[k * 2 + i] = scratchpad[i];
  }

  //    Serial.println("");

}






void readAirTemp() {

  Serial.println("\n================= READING DS18B20 AIR TEMPERATURE SENSOR ================= ");
  Serial.println("Fetching air temperature...");
  air_temp.requestTemperatures();
  at.reset(); // reset one-wire bus

  pixels.setBrightness(50);
  pixels.setPixelColor(0, pixels.Color(0,255,234));
  pixels.show();
  Serial.println("Getting temperature bytes...");
  at.write(0xCC); // skip ROM
  at.write(0xBE); // read scratchpad command
  byte scratchpad[8];
  for (int i = 0; i < 2; i++) { // step first two bytes, write them to scratchpad
    scratchpad[i] = at.read();
    //    Serial.print(scratchpad[i], BIN);
    //    Serial.print(" ");
  }

  for (int i = 0; i < 2; i++) {
    airTempArray[i] = scratchpad[i];
  }

  printAirTemperature();
  pixels.setBrightness(0);
  pixels.show();
}

void loop() {}


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
