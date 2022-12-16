// Development code for implementing the temperature conversion from binary to degrees Celcius. 
// Note: this process is different than that of the Dallas Temperature library. The bitmap applied is taken from the DS28EA00 datasheet.

// Written 6 December 2022 by Cameron Planck

void setup() {}

void loop() {

//  byte MSB = 0b00010001;
//  byte LSB = 0b01111010; 


//  // -10.125 (good)
//  byte MSB = 0b11111111; 
//  byte LSB = 0b01011110;

//  // 25.0625 (good)
//  byte MSB = 0b00000001; 
//  byte LSB = 0b10010001;
  
//  // 85
//  byte MSB = 0b00000101; 
//  byte LSB = 0b01010000;

  // -40
  byte MSB = 0b11111101; 
  byte LSB = 0b10000000;

  float temp = decodeTemperatureBytes(LSB, MSB);
  Serial.println(temp);
  delay(1000);

}

float decodeTemperatureBytes(byte LSB, byte MSB){
  
  uint16_t binInt = 0b1000000000000000;

  binInt = binInt | LSB;
  binInt = binInt | (MSB << 8);

  // if leading bit of MSB is 1, undo two's complement (subtract 1 and invert)
  float sign = 1;
  if(bitRead(MSB, 7)){
    binInt = binInt - 1;
    binInt = ~binInt;
    sign = -1;
    }

  float temp = 0;
  int counter = 0;
  for(int i = -4; i<= 6; i++){
    temp = temp + (bitRead(binInt,counter)*pow(2,i));
    counter++;
    } 
  
  return (sign*temp);
  }



//float decodeTemperatureBytes(byte LSB, byte MSB){
//
//  int bitArray[16];
//
//  int k = 0;
//  for(int i = 7; i>=0; i=i-1){
//     int bits = bitRead(MSB,i);
//     bitArray[k] = bits;
//     k++;
//    }
//  
//  for(int i = 7; i>=0; i=i-1){
//     int bits = bitRead(LSB,i);
//     bitArray[k] = bits;
//     k++;
//    }
//
//   float temperature = (bitArray[5]*pow(2,6) + bitArray[6]*pow(2,5) + bitArray[7]*pow(2,4) + bitArray[8]*pow(2,3) + bitArray[9]*pow(2,2) + bitArray[10]*pow(2,1) + bitArray[11]*pow(2,0) + bitArray[12]*pow(2,-1) + bitArray[13]*pow(2,-2) + bitArray[14]*pow(2,-3) + bitArray[15]*pow(2,-4)); 
//   
//   if(bitArray[4] == 1){
//    temperature = -temperature;
//    }
//   
//   return(temperature);
//}

  
