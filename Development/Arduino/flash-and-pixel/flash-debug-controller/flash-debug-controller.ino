// Same code as fetch-data-slave-ROM-store, but optimized to work with both TOP and BOTTOM temperature strings

// Hardware: a Feather M0 connected to the SIMB3 mainboard over 5V I2C. The one-wire wire devices should be connected over pins 12, 13.

// Written 14 January, 2023 by Cameron Planck

#include "Wire.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "Adafruit_NeoPixel.h"
#include "Express_M0_Flash.h"

bool eventReceived = false;

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
  pixels.begin();
  pixels.setBrightness(0);
  Wire.onReceive(receiveEvent);
}


void receiveEvent(int numBytes) {
  eventReceived = true;
}

void loop() {
  if(eventReceived){
    if (digitalRead(ROM_SEARCH_ENABLE) == HIGH) {
      writeROMS();
      eventReceived = false;
    }
    else{
      readROMS();
      readROMS();
      readROMS();
      eventReceived = false;
    }
  }
}

void writeROMS(){

  pixels.setBrightness(50);
  pixels.setPixelColor(0, pixels.Color(121,0,255));
  pixels.show();
  
  Serial.println("ROM SEARCH ENABLED. DISCOVERING ROMS FOR STRINGS...");
  onBoardFlash.format();
  byte payload[3] = {0b11101110, 129, 0b10011000};
  onBoardFlash.writebytes("TOPSTRING_ROMS.csv", payload, 3);
  Serial.println("Sucessfully wrote TOPSTRING_ROMS.csv");

  pixels.setBrightness(0);
  pixels.show();
}

void readROMS(){

  pixels.setBrightness(50);
  pixels.setPixelColor(0, pixels.Color(14,131,255));
  pixels.show();

  byte buffer[3];
  onBoardFlash.initialize();
  onBoardFlash.readbytes("TOPSTRING_ROMS.csv", buffer, 3);
  Serial.print("ROM buffer: ");
  Serial.println(buffer[0], DEC);
  Serial.println(buffer[1], DEC);
  Serial.println(buffer[2], DEC);

  pixels.setBrightness(0);
  pixels.show();

}