// SIMB3 One-Wire Benchtop Validation Code.
// This code is for reading the custom SIMB3 One-Wire Temperature String boards (currently produced by PCBway). 
// It performs the discovery loop, records all the addresses in order, and then loop over the addresses to collect and print the temperatures.
// Written 15 November 2022 by Cameron Planck

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

// array carrying device addresses. For one board it should be 8*20 = 160
byte ROMarray[640];
byte present = 0;


void chipDiscover(int k){

     // this function applies the chain command and control sequence, 

    byte addr[8];
    int i;
    
//    Serial.println("------------------ CHIP DISCOVER BEGIN --------------------");

//    Serial.print("The value of K is: ");
//    Serial.println(k);
  
    // reset 1-wire bus
    present = ds.reset();
//    Serial.print("Reseting again. Chip still present?: ");
//    Serial.println(present);
  
    // execute conditional read ROM command and print response (64-bit registration number)
//    Serial.println("Applying Conditional Read ROM Command...");
    ds.write(0x0F);
//    Serial.print("Response: ");
    for (i = 0; i < 8; i++) { // step through each bit, write it to ROMarray
      addr[i] = ds.read();
//      Serial.print(addr[i], HEX);
//      Serial.print(" ");
      ROMarray[k*8 + i] = addr[i];
    }
    
    // reset i-wire bus
    present = ds.reset();
//    Serial.println();
//    Serial.print("Reseting again. Chip still present?: ");
//    Serial.println(present);
  
    // execute match ROM to select conditional chip
//    Serial.println("Applying Match ROM Command...");
    ds.write(0x55);
    for (i = 0; i < 8; i++) {          
      ds.write(addr[i]);
//      Serial.print(addr[i], HEX);
//      Serial.print(" ");
    }
//    Serial.println("");
  
//    Serial.print("Applying Chain Done command. Result: ");
//    Serial.println(sensors.chainDone(), HEX); // use this to print the value of the chaindDone method
    sensors.chainDone(); // if not printing, then call this
    
  }




void loop(void){ 

  // device count from Dallas Temperature library
  // note this number might include other 1-wire chips on board in addition to the DS28AE00s
  // a better way to end the discovery processs is to run the discovery loop until <8 Bytes FFh>
  Serial.print("Number of 1-wire chips discovered on bus: ");
  Serial.println(sensors.getDeviceCount()); 

  present = ds.reset();
  Serial.print("Present?: ");
  Serial.println(present);

  // turn on chain mode
  Serial.print("Turning on chain mode. Result: ");
  Serial.println(sensors.chainOn(), HEX);

  // loop over chips, applying Conditional ROM command and writing response to ROMarray
  for(int i=0; i<80; i++){
    chipDiscover(i);
  }

  // fetch temperatures from each thermistor by indexing ROMarray
  sensors.requestTemperatures();
  for(int i = 0; i<80; i++){
    DeviceAddress thermistor = {ROMarray[i*8], ROMarray[i*8 +1], ROMarray[i*8 +2], ROMarray[i*8 +3], ROMarray[i*8 +4], ROMarray[i*8 +5], ROMarray[i*8 +6], ROMarray[i*8 +7]};
//    sensors.requestTemperaturesByAddress(thermistor);
    Serial.print("THERMISTOR ");
    Serial.print(i+1);
    Serial.print(" READING: ");
    Serial.println(sensors.getTempC(thermistor));

  }
  
}
