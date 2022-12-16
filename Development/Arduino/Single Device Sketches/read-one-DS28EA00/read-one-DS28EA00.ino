// Development sketch for testing communication between ONE DS28EA00 temperature chip and Arduino.
// The raw binary is read off the DS28EA00, bit shifted, and manually converted to a temperature value. 
// NOTE: the temperature conversion process used in this code is from the Dallas Temperature Library is is not documented in the DS28EA datasheet
// See decode-temperature.ino sketch for correct process

// Hardware: an Arduino Feather m0 with a single DS28EA00 powered with data flowing through pin 12. 

// Written 1 December, 2022 by Cameron Planck

#include <Wire.h>       
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire  ds(12);  // on pin 12 (a 4.7K (ish) resistor is necessary, it wont work without it)

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&ds);


void setup(){
  // Start serial communication for debugging purposes
  Serial.begin(9600);
  sensors.begin();
}


byte ROMarray[40];
byte present = 0;

void loop(){

    // check number of one-wire devices present on bus
    Serial.print("Number of one-wire devices detected: ");
    Serial.println(sensors.getDeviceCount());

    // ask all chips to perform temperature conversion
    sensors.requestTemperatures();

    // get temperature bits by reading scratchpad
    present = ds.reset();
    Serial.print("Present?: ");
    Serial.println(present);

    // turn chain mode on
    sensors.chainOn();

    // run chip discover process to generate ROMarray
    chipDiscover(0);

    // write read scratchpad command and read in the 8 bytes from the scratchpad. Note that only the first two are needed for temperature)
    Serial.println("Reading Scratchpad...");
    ds.write(0xBE); // read scratchpad command
    byte scratchpad[8];
    for (int i = 0; i < 8; i++) { // step through each byte, write it to ROMarray
      scratchpad[i] = ds.read();
      Serial.print(scratchpad[i], HEX);
      Serial.print(" ");
    }


    // inspect contents of scratchpad
    Serial.println("");
    Serial.println("BINARY: ");
    Serial.print("MS Byte: ");
    Serial.println((int16_t) scratchpad[1], BIN);
    Serial.print("LS Byte: ");
    Serial.println((int16_t) scratchpad[0], BIN);

    // check if value is negative by looking at signed bits (0-4) in MSbyte. 
    int32_t neg = 0x0;
    if (scratchpad[1] & 0x80) // if the 
      neg = 0xFFF80000;

    // shift and stack bits in MSbyte and LSbyte to form combined binary number
    int32_t fpTemperature = 0;
    fpTemperature = (((int16_t) scratchpad[1]) << 11)
                    | (((int16_t) scratchpad[0]) << 3)
                    | neg;

    // print combined binary number
    Serial.print("Combined MS Byte + LS Byte: ");
    Serial.println(fpTemperature, BIN);

    // manually calculate temperature from raw
    float convertedTemp = fpTemperature * 0.0078125f;
    Serial.print("MANUALLY CALCULATED TEMP:");
    Serial.println(convertedTemp);

    // calculate temperature from library and compare
    Serial.print("LIBRARY CALCULATED TEMP:");
    float temp = sensors.getTempCByIndex(0);
    Serial.println(temp); 
    Serial.println("");

    delay(1000);

  }



void chipDiscover(int k){

    // note: sensors.chainOn() must be called before the chipDiscover routine
    byte addr[8];
    byte i;
    
    Serial.println("------------------ CHIP DISCOVER BEGIN --------------------");
  
    // reset 1-wire bus
    present = ds.reset();
    Serial.print("Reseting again. Chip still present?: ");
    Serial.println(present);
  
    // execute conditional read ROM command and print response (64-bit registration number)
    Serial.println("Applying Conditional Read ROM Command...");
    ds.write(0x0F);
    Serial.print("Response: ");
    for (i = 0; i < 8; i++) {          
      addr[i] = ds.read();
      Serial.print(addr[i], HEX);
      Serial.print(" ");
      ROMarray[k*8 + i] = addr[i];
    }
    
    // reset i-wire bus
    present = ds.reset();
    Serial.println();
    Serial.print("Reseting again. Chip still present?: ");
    Serial.println(present);
  
    // execute match ROM to select conditional chip
    Serial.println("Applying Match ROM Command...");
    ds.write(0x55);
    for (i = 0; i < 8; i++) {          
      ds.write(addr[i]);
      Serial.print(addr[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
  }

  void matchROM(){

     // can optionally use this instead of the chipDiscover() routine
     
    // execute match ROM to select conditional chip with known address (for testing). 
    Serial.println("Applying Match ROM Command...");
    byte addr[8] = {0x42,0xF7,0x70,0x68, 0, 0, 0, 0xAB};
    ds.write(0x55); // match ROM command
    for (int i = 0; i < 8; i++) {          
      ds.write(addr[i]);
      Serial.print(addr[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
    
    }

  
