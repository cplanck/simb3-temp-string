// Development code to write and read data from the Adafruit Feather M0 Express onboard flash chip
// Code used the Adafruit_Express_Flash library (by CJP) to format, read, and write bytes to the Feather M0 card

#include <Adafruit_Express_Flash.h>
Adafruit_Express_Flash onBoardFlash;

// number of bytes to read/write from flash memory
#define BYTENUM 4

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Adafruit SPI Flash FatFs Simple Datalogging Example");
  onBoardFlash.initialize();
}

void loop() {
    
  // erase flash
  onBoardFlash.format();

  // write ROMs to SPI flash
  byte ROMs[BYTENUM] = {0b01111110, 0b11011001, 0b10111101, 0b10011100};
  onBoardFlash.writebytes("TOPSTRING_ROMS.csv", ROMs, BYTENUM);

  // read ROMs from onboard SPI flash and write to ROM_return buffer and print
  byte ROM_return[BYTENUM];
  int suc_read = onBoardFlash.readbytes("TOPSTRING_ROMS.csv", ROM_return, BYTENUM);

  Serial.println("ROM RETURN:");
  for (int i = 0; i < BYTENUM; i++) {
    Serial.println(ROM_return[i], BIN);
  }

  // print verification bytes
  Serial.print("\nFLASHCHECK: ");
  Serial.println(suc_read);

  delay(2000);
}
