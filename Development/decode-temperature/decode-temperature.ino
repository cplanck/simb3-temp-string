void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

//  byte MSB = 0b00010001;
//  byte LSB = 0b01111010; 

  byte MSB = 0b10001;
  byte LSB = 0b01111010; 

//decodeTemperatureBytesDS(LSB, MSB);
  
  Serial.println(decodeTemperatureBytes(LSB, MSB));
  delay(1000);
}

//float decodeTemperatureBytes(byte LSB, byte MSB){
//
//    int32_t neg = 0x0;
//    if (MSB & 0x80) // if the 
//      neg = 0xFFF80000;
//  
//    // shift and stack bits in MSbyte and LSbyte to form combined binary number
//    int32_t fpTemperature = 0;
//    fpTemperature = (((int16_t) MSB) << 11)
//                    | (((int16_t) LSB) << 3)
//                    | neg;
//  
//    // manually calculate temperature from raw
//    float convertedTemp = fpTemperature * 0.0078125f;
//
//    return(convertedTemp);
//}




float decodeTemperatureBytes(byte LSB, byte MSB){

  int bitArray[16];

  int k = 0;
  for(int i = 7; i>=0; i=i-1){
     int bits = bitRead(MSB,i);
     bitArray[k] = bits;
     k++;
    }
  
  Serial.print(" ");
  for(int i = 7; i>=0; i=i-1){
     int bits = bitRead(LSB,i);
     bitArray[k] = bits;
     k++;
    }

  for(int i=0; i<16; i++){
    Serial.print(bitArray[i]);
    Serial.print(" ");
    }

   Serial.println("");

   float temperature = (bitArray[5]*pow(2,6) + bitArray[6]*pow(2,5) + bitArray[7]*pow(2,4) + bitArray[8]*pow(2,3) + bitArray[9]*pow(2,2) + bitArray[10]*pow(2,1) + bitArray[11]*pow(2,0) + bitArray[12]*pow(2,-1) + bitArray[13]*pow(2,-2) + bitArray[14]*pow(2,-3) + bitArray[15]*pow(2,-4)); 
   return(temperature);
}
