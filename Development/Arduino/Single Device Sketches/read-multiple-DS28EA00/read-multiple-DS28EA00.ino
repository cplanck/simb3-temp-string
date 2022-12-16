// Development sketch for testing communication between multiple DS28EA00 temperature chips and Arduino.
// Th raw binary is read off the DS28EA00s, bit shifted, and manually converted to temperature values. 
// the full 2 temperature bytes are also appended to a temperature array, which is later to be streamed to the SIMB3 main board via i2c
// NOTE: the temperature conversion process used in this code is from the Dallas Temperature Library is is not documented in the DS28EA datasheet
// See decode-temperature.ino sketch for correct process

 
// Hardware: Arduino Feather m0 with a multiple DS28EA00s chained together, powered, and flowing data through pin 12. 

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

// number of DS28EO00s to be read
int chipNum = 80;

// define ROM array, which needs to be at minimum 8*(number of DS28EA00s)
byte ROMarray[640];

// define tempArray which needs to be at least 2*(number of DS28EA00s)
byte tempArray[160];

byte present = 0;

void loop(){

    // check number of one-wire devices present on bus
    Serial.println("------------------ INITIALIZE ONE-WIRE --------------------");
    Serial.print("Number of one-wire devices detected: ");
    Serial.println(sensors.getDeviceCount());

    // ask all chips to perform temperature conversion
    sensors.requestTemperatures();

    present = ds.reset();
    Serial.print("Present?: ");
    Serial.println(present);

    // turn chain mode on
    sensors.chainOn();

    // run chip discover process to generate ROMarray
    Serial.println("------------------ CHIP DISCOVER BEGIN --------------------");
    for(int i = 0; i < chipNum; i++){
      chipDiscover(i);
    };

    // loop over ROMarray, select each chip, ask for temperature, and write to tempArray
    Serial.println("------------------ CHIP SELECTION BEGIN -------------------");
    for(int i = 0; i < chipNum; i++){
      ds.reset();
      chipSelect(i);
    }

     Serial.println("---------------- CONVERTED TEMPERATURES -----------------");
     convertTemperatures();
     
    delay(1000);
  }


void chipDiscover(int k){

    // note: sensors.chainOn() must be called before the chipDiscover routine
    byte addr[8];
    byte i;
      
    // reset 1-wire bus
    present = ds.reset();
    Serial.print("Reseting again. Chip still present?: ");
    Serial.println(present);
  
    // execute conditional read ROM command and print response (64-bit registration number)
    Serial.println("Applying Conditional Read ROM Command...");
    ds.write(0x0F);
    Serial.print("Chip Address: ");
    for (i = 0; i < 8; i++) {          
      addr[i] = ds.read();
      Serial.print(addr[i], HEX);
      Serial.print(" ");
      ROMarray[k*8 + i] = addr[i];
    }

    Serial.println("");

    // turn chain mode off
    sensors.chainDone();
 
  }


void chipSelect(int k){

    // apply Match ROM command and select chip by writing ROM
    Serial.println("Selecting chip by ROM...");
    byte addr[8];
    ds.write(0x55);
    for (int i = 0; i < 8; i++) {          
      addr[i] = ROMarray[k*8 + i];
      ds.write(addr[i]);
      Serial.print(addr[i], HEX);
      Serial.print(" ");
      }

    Serial.println("");
    
    Serial.println("Reading Scratchpad...");
    ds.write(0xBE); // read scratchpad command
    byte scratchpad[2];
    Serial.print("Scratchpad Bytes: ");
    for (int i = 0; i < 2; i++) { // step through each byte, write it to ROMarray
      scratchpad[i] = ds.read();
      Serial.print(scratchpad[i], BIN);
      Serial.print(" ");
    }
    Serial.println("");

    // add MSB and LSB to tempArray
    for (int i = 0; i < 2; i++) {          
      tempArray[k*2 + i] = scratchpad[i];
      }

    Serial.println("");

  }


  void convertTemperatures(){
    
     int k = 0;
     for(int i = 0; i < chipNum*2; i=i+2){
            
      // check if value is negative by looking at signed bits (0-4) in MSbyte. 
      int32_t neg = 0x0;
      if (tempArray[i+1] & 0x80) // if the 
        neg = 0xFFF80000;
  
      // shift and stack bits in MSbyte and LSbyte to form combined binary number
      int32_t fpTemperature = 0;
      fpTemperature = (((int16_t) tempArray[i+1]) << 11)
                      | (((int16_t) tempArray[i]) << 3)
                      | neg;
  
      // manually calculate temperature from raw binary value (fpTemperature)
      float convertedTemp = fpTemperature * 0.0078125f;
      Serial.print("CHIP ");
      Serial.print(k+1); // +1 to align it with the chip numbers on the boards
      Serial.print(" TEMPERATURE: ");
      Serial.print(convertedTemp);
      Serial.println(" (C)");
      k++;
      }
  }

  
