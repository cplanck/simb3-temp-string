// I2C slave code for fetching temperature string values and sending them back to master in 32-byte chunks

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
int state;

// index variable to indicate which packet to write
int packetIndex = 0;

// number of DS28EO00s to be read
# define chipNum 80

// define ROM array, which needs to be at minimum 8*(number of DS28EA00s)
byte ROMarray[8*chipNum];

// define tempArray which needs to be at least 2*(number of DS28EA00s)
byte tempArray[2*chipNum];

// define airTempArray which holds the two bytes from the DS18B20 air temperature sensor
byte airTempArray[2];

byte present = 0;

// 32-byte short buffer for i2c transmission
byte tempPacket[32];

// called when master sends data
void receiveEvent(int numBytes){
  while(Wire.available()){
    state = Wire.read();
    packetIndex = Wire.read();
  }
}

// called when master asks for data
void requestEvent(){
  if(state == 1 && packetIndex == 0){
      readTopString();
      packTempPacket(packetIndex);
      Wire.write(tempPacket, 32);  
      packetIndex++;
    }

   else if(state == 1){
      packTempPacket(packetIndex);
      Wire.write(tempPacket, 32);
      packetIndex++;
    }
    else if(state == 2){
      readAirTemp();
      Wire.write(airTempArray, 2);
    }
}


// function to grab a subset of data from the temperature string buffer. 
void packTempPacket(int ind){
   for(int i=0; i < 32; i++){
      tempPacket[i] = tempArray[ind*32 + i];
   }
}
 

void readTopString(){

    Serial.println("===================== READING TOP TEMPERATURE STRING ===================== ");
    Serial.println("Initialize One-Wire....");
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
    Serial.println("Beginning chip discovery...");
    for(int i = 0; i < chipNum; i++){
      chipDiscover(i);
    };

    // loop over ROMarray, select each chip, ask for temperature, and write to tempArray
    Serial.println("Getting temperature bytes...");
    for(int i = 0; i < chipNum; i++){
      ds.reset();
      chipSelect(i);
    }

    printTemperatureString();
  
  }

void chipDiscover(int k){

    // note: sensors.chainOn() must be called before the chipDiscover routine
    byte addr[8];
    byte i;
      
    // reset 1-wire bus
    present = ds.reset();
//    Serial.print("Reseting again. Chip still present?: ");
//    Serial.println(present);
  
    // execute conditional read ROM command and print response (64-bit registration number)
//    Serial.println("Applying Conditional Read ROM Command...");
    ds.write(0x0F);
//    Serial.print("Chip Address: ");
    for (i = 0; i < 8; i++) {          
      addr[i] = ds.read();
//      Serial.print(addr[i], HEX);
//      Serial.print(" ");
      ROMarray[k*8 + i] = addr[i];
    }

//    Serial.println("");

    // turn chain mode off
    sensors.chainDone();
 
  }

void chipSelect(int k){

    // apply Match ROM command and select chip by writing ROM
//    Serial.println("Selecting chip by ROM...");
    byte addr[8];
    ds.write(0x55);
    for (int i = 0; i < 8; i++) {          
      addr[i] = ROMarray[k*8 + i];
      ds.write(addr[i]);
//      Serial.print(addr[i], HEX);
//      Serial.print(" ");
      }

//    Serial.println("");
    
//    Serial.println("Reading Scratchpad...");
    ds.write(0xBE); // read scratchpad command
    byte scratchpad[2];
//    Serial.print("Scratchpad Bytes: ");
    for (int i = 0; i < 2; i++) { // step through each byte, write it to ROMarray
      scratchpad[i] = ds.read();
//      Serial.print(scratchpad[i], BIN);
//      Serial.print(" ");
    }
//    Serial.println("");

    // add MSB and LSB to tempArray
    for (int i = 0; i < 2; i++) {          
      tempArray[k*2 + i] = scratchpad[i];
      }

//    Serial.println("");

  }

void readAirTemp(){

  Serial.println("\n================= READING DS18B20 AIR TEMPERATURE SENSOR ================= ");
  Serial.println("Fetching air temperature...");
  air_temp.requestTemperatures();
  at.reset(); // reset one-wire bus
 
  Serial.println("Getting temperature bytes...");
  at.write(0xCC); // skip ROM
  at.write(0xBE); // read scratchpad command
  byte scratchpad[8];
  for (int i = 0; i < 2; i++) { // step first two bytes, write them to scratchpad
    scratchpad[i] = at.read();
//    Serial.print(scratchpad[i], BIN);
//    Serial.print(" ");
  }

  for (int i = 0; i < 2; i++){          
      airTempArray[i] = scratchpad[i];
   }

   printAirTemperature();
}

 void loop(){}


 void printAirTemperature(){

   Serial.println("CONVERTED AIR TEMPERATURE VALUES (DEG C): ");
   float air_temp = decodeTemperatureBytes(airTempArray[0], airTempArray[1]);
   Serial.print(air_temp);
   Serial.println(" ");
}

void printTemperatureString(){

   Serial.println(F("CONVERTED TEMPERATURE STRING VALUES (DEG C): "));
   int k = 1;
   for(int i = 0; i<chipNum*2; i=i+2){
      float ow_dtc_value = decodeTemperatureBytes(tempArray[i], tempArray[i+1]);
      Serial.print(k);
      Serial.print(": ");
      Serial.print(ow_dtc_value);
      Serial.print(" ");
      if(i%8 == 0 & i!=0){
        Serial.println("");
        }
       k++;
    }
    Serial.println("");
}

float decodeTemperatureBytes(byte LSB, byte MSB){

  int bitArray[16];

  int k = 0;
  for(int i = 7; i>=0; i=i-1){
     int bits = bitRead(MSB,i);
     bitArray[k] = bits;
     k++;
    }
  
  for(int i = 7; i>=0; i=i-1){
     int bits = bitRead(LSB,i);
     bitArray[k] = bits;
     k++;
    }

   float temperature = (bitArray[5]*pow(2,6) + bitArray[6]*pow(2,5) + bitArray[7]*pow(2,4) + bitArray[8]*pow(2,3) + bitArray[9]*pow(2,2) + bitArray[10]*pow(2,1) + bitArray[11]*pow(2,0) + bitArray[12]*pow(2,-1) + bitArray[13]*pow(2,-2) + bitArray[14]*pow(2,-3) + bitArray[15]*pow(2,-4)); 
   return(temperature);
}


 
