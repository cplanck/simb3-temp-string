#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>


// Configuration of the flash chip pins and flash fatfs object.
// You don't normally need to change these if using a Feather/Metro
// M0 express board.
#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV  // Flash chip type.
                                              // If you change this be
                                              // sure to change the fatfs
                                              // object type below to match.

#if defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS)
  #define FLASH_SS       SS                    // Flash chip SS pin.
  #define FLASH_SPI_PORT SPI                   // What SPI port is Flash on?
  #define NEOPIN         8
#else
  #define FLASH_SS       SS1                    // Flash chip SS pin.
  #define FLASH_SPI_PORT SPI1                   // What SPI port is Flash on?
  #define NEOPIN         40
#endif

Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);     // Use hardware SPI

Adafruit_W25Q16BV_FatFs fatfs(flash);

#define FILE_NAME      "TOPSTRING_ROMS.csv"


void setup() {
  // Initialize serial port and wait for it to open before continuing.
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("Adafruit SPI Flash FatFs Simple File Printing Example");

  // Initialize flash library and check its chip ID.
  if (!flash.begin(FLASH_TYPE)) {
    Serial.println("Error, failed to initialize flash chip!");
    while(1);
  }
  Serial.print("Flash chip JEDEC ID: 0x"); Serial.println(flash.GetJEDECID(), HEX);

  // First call begin to mount the filesystem.  Check that it returns true
  // to make sure the filesystem was mounted.
  if (!fatfs.begin()) {
    Serial.println("Error, failed to mount newly formatted filesystem!");
    Serial.println("Was the flash chip formatted with the fatfs_format example?");
    while(1);
  }
  Serial.println("Mounted filesystem!");

  File dataFile = fatfs.open(FILE_NAME, FILE_READ);
  if (dataFile) {
    // File was opened, now print out data character by character until at the
    // end of the file.
    Serial.println("Opened file, printing contents below:");
    while (dataFile.available()) {
      // Use the read function to read the next character.
      // You can alternatively use other functions like readUntil, readString, etc.
      // See the fatfs_full_usage example for more details.
      char c = dataFile.read();
      Serial.println(c, BIN);
    }
  }
  else {
    Serial.println("Failed to open data file! Does it exist?");
  }
}

void loop() {
  // Nothing to do in main loop.
  delay(100);
}
