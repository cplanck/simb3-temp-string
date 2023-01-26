// DS28AE00 Breadboard Code.
// This code is for reading DS29AE00 temperature chips in chain-mode. 
// It performs the discovery loop, records all the addresses in order, and then loop over the addresses to collect and print the temperatures.
// Written 14 July 2022 by Cameron Planck
// Last updated: 11/15/2022

// NOTE: this is the code used for the initial validation of the SIMB3 One-Wire Digital Temperature String. It formed the basis for the ow-tempstring-benchtop code used for validating the temperature string boards.

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire  ds(12);  // on pin 12 (a 4.7K resistor is necessary, it wont work without it)

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&ds);

void setup(void) {
   // Start serial communication for debugging purposes
  Serial.begin(9600);
  // Start up the library
  sensors.begin();
  
}

// array carrying device addresses. It should be size 8*N_dev
byte ROMarray[40];
byte present = 0;


void chipDiscover(int k){

     // this function applies the chain command and control sequence, 

    Serial.print("The value of K is: ");
    Serial.println(k);

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
    
//    delay(1000);
  
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
    }
    Serial.println("");
  
    Serial.print("Applying Chain Done command. Result: ");
    Serial.println(sensors.chainDone(), HEX);
    
  }




void loop(void){ 

  // device count from Dallas Temperature library
  // note this number might include other 1-wire chips on board in addition to the DS28AE00s
  // a better way to end the discovery processs is to run the discovery loop until <8 Bytes FFh>
//  Serial.print("Number of 1-wire chips discovered on bus: ");
//  Serial.println(sensors.getDeviceCount()); 

  present = ds.reset();
  Serial.print("Present?: ");
  Serial.println(present);

  // turn on chain mode
  Serial.print("Turning on chain mode. Result: ");
  Serial.println(sensors.chainOn(), HEX);

  // loop over chips, applying Conditional ROM command and writing response to ROMarray
  for(int i=0; i<5;i++){
    chipDiscover(i);
  }

  // fetch temperatures from each thermistor by indexing ROMarray
  sensors.requestTemperatures();
  for(int i = 0; i<5; i++){
    DeviceAddress thermistor = {ROMarray[i*8], ROMarray[i*8 +1], ROMarray[i*8 +2], ROMarray[i*8 +3], ROMarray[i*8 +4], ROMarray[i*8 +5], ROMarray[i*8 +6], ROMarray[i*8 +7]};
//    sensors.requestTemperaturesByAddress(thermistor);
    Serial.print("THERMISTOR ");
    Serial.print(i+1);
    Serial.print(" READING: ");
    Serial.println(sensors.getTempC(thermistor));
  }
  
}
