// SIMB3 One-Wire Controller Code (development)
// This code is for the SIMB3 One-Wire control board (Adafruit Feather m0) connected to the SIMB3 mainboard via 5V I2C.
// Upon power up, this code listens for I2C commands issued by the SIMB3 control board. It then reads the one-wire devices requested and 
// writes the data back to master. 

// hardware: Feather M0 connected to an SIMB3 mainboard over I2C and to one-wire devices over pins 12, 13.

// written 5 December, 2022 by Cameron Planck

#include <Wire.h>       
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire  ds(12);  // on pin 12 (a 4.7K (ish) resistor is necessary, it wont work without it)
OneWire  at(13);  //  on pin 13 (a 4.7K (ish) resistor is necessary, it wont work without it)

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&ds);
DallasTemperature air_temp(&at);

void setup(){
  Serial.begin(9600);

  // initialize I2C with address 1
  Wire.begin(1);   

  // runs when master sends data                            
  Wire.onReceive(receiveEvent);

  // runs when master asks for data
  Wire.onRequest(requestEvent);

  // start one-wire devices
  sensors.begin();
  air_temp.begin();
  
}

// state variable set by what master has requested
int state = 0;

// number of DS28EO00s to be read
# define chipNum 80

// define ROM array, which needs to be at minimum 8*(number of DS28EA00s)
byte ROMarray[8*chipNum];

// define tempArray which needs to be at least 2*(number of DS28EA00s)
byte tempArray[2*chipNum];

byte present = 0;

// called when master sends data
void receiveEvent(int numBytes){
  while(Wire.available()){
    Serial.println("DATA FROM MASTER RECEIVED");
    state = Wire.read(); // update state with whatever master sent
    Serial.print(state); // maybe this is the problem? Calling a serial command inside an i2c method
  }
}

// called when master asks for data
void requestEvent(){
  Serial.println("SENDING DATA TO MASTER");
  if(state == 1){
      readTopString();
      Wire.write(tempArray, 160);  
    }
   else if(state == 2){
    readAirTemp();
    Wire.write(tempArray, 2);
    }
  }

 void readAirTemp(){

  air_temp.requestTemperatures();

  at.reset(); // reset one-wire bus
 
  Serial.println("Reading Scratchpad...");
  at.write(0xCC); // skip ROM
  at.write(0xBE); // read scratchpad command
  byte scratchpad[8];
  for (int i = 0; i < 2; i++) { // step first two bytes, write them to scratchpad
    scratchpad[i] = at.read();
    Serial.print(scratchpad[i], BIN);
    Serial.print(" ");
  }
  
// float temp = air_temp.getTempCByIndex(0);
//  Serial.println(temp); 
//  Serial.println("");

  for (int i = 0; i < 2; i++) {          
        tempArray[i] = scratchpad[i];
        }
}

void readTopString(){

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
    Serial.println("------------------ GET TEMPERATURE BITS -------------------");
    for(int i = 0; i < chipNum; i++){
      ds.reset();
      chipSelect(i);
    }
  
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

 void loop(){}


 
