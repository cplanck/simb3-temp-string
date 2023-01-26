// Test code for validating single or multiple Cryosphere Innovation DS28EA00 one-wire temperature string boards.
// The code detects the number of one-wire chips on the bus, looping over each and writing its temperature to the console. 
// Note: this is just test code with minimal console.logs. For a more detailed readout, use Development/read-multiple-DS29EA00s.ino

// Hardware: the one-wire temperature string test fixture. 

// Written 19 January, 2023 by Cameron Planck

 
#include <Wire.h>       
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire  ds(11);  // on pin 12 (a 4.7K (ish) resistor is necessary, it wont work without it)

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&ds);


void setup(){
  // Start serial communication for debugging purposes
  Serial.begin(9600);
  sensors.begin();
  while(!Serial);
}

// define ROM array, which needs to be at minimum 8*(number of DS28EA00s)
byte ROMarray[640];

// define tempArray which needs to be at least 2*(number of DS28EA00s)
byte tempArray[160];

void loop(){

    // check number of one-wire devices present on bus
    Serial.println("------------------ BEGINNING SCAN --------------------");
    Serial.print("\nNumber of DS28EA00 sensors detected: ");
    sensors.begin(); // call sensors.begin() in loop to allow hot-plugging for testing
    Serial.println(sensors.getDeviceCount());
    Serial.println("");
    int sensorCount = sensors.getDeviceCount();

    sensors.requestTemperatures();

    sensors.chainOn();

    for(int i = 0; i < sensorCount; i++){
      chipDiscover(i);
    };

    for(int i = 0; i < sensorCount; i++){
      ds.reset();
      chipSelect(i);
    }

     Serial.println("Converted Temperatures (deg C):");
     int k = 0;
     int count = 1;
     while(k < 2*sensorCount){
        float temp = decodeTemperatureBytes(tempArray[k], tempArray[k+1]);
        Serial.print("Sensor ");
        Serial.print(count);
        Serial.print(": ");
        Serial.println(temp);
        k=k+2;
        count++;
      }
     
    delay(1000);
  }


void chipDiscover(int k){

    byte addr[8];
    byte i;
      
    // reset 1-wire bus
    ds.reset();

    // execute conditional read ROM command and print response (64-bit registration number)
    ds.write(0x0F);
    for (i = 0; i < 8; i++) {          
      addr[i] = ds.read();
      ROMarray[k*8 + i] = addr[i];
    }

    sensors.chainDone();
 
  }


void chipSelect(int k){

    byte addr[8];
    ds.write(0x55);
    for (int i = 0; i < 8; i++) {          
      addr[i] = ROMarray[k*8 + i];
      ds.write(addr[i]);
      }

    ds.write(0xBE); // read scratchpad command
    byte scratchpad[2];
    for (int i = 0; i < 2; i++) { // step through each byte, write it to ROMarray
      scratchpad[i] = ds.read();
    }

    // add MSB and LSB to tempArray
    for (int i = 0; i < 2; i++) {          
      tempArray[k*2 + i] = scratchpad[i];
      }

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

  
