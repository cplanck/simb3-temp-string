// Development code for the bit packing algorithm for the SIMB3 one-wire temperature string
// IMPORTANT: this code will (currently) only work for EVEN values of temperature chips, i.e., readings from 2, 20, or 40 chips, NOT 3 or 41
// this is due to the symmetry invoked when packing the array. 
// NOTE: this code heavily references the map in the Excel/BitPacking.xlsx document. It will be hard to understand without it. 

// Hardware: this code runs on any Arduino. It doesn't actually do anything except crunch numbers and export them to the serial monitor.

// Written 3 December 2022 by Cameron Planck

void setup(){
  
  }

void loop() {

  #define NumDS28AE00s 4
  #define PackedBytesLength NumDS28AE00s * 2 * 3/4

  byte receiveBuffer[NumDS28AE00s*2] = {0b10001010, 0b00000010, 0b10001010, 0b00000010, 0b11111010, 0b00001100, 0b11001010, 0b00001011};

  // packed byte array. it gets passed to packTemperaturesForTransmission() to be filled.
  byte packedBytes[PackedBytesLength];


  Serial.println("VALUES BEFORE PACKING: ");
  for(int i = 0; i < NumDS28AE00s*2; i++){
     Serial.print(receiveBuffer[i], BIN);
     Serial.print(" ");
    }
  Serial.println("\n");
  
  packTemperaturesForTransmission(NumDS28AE00s, receiveBuffer, packedBytes);

  Serial.println("PACKED VALUES: ");
  for(int i = 0; i < PackedBytesLength; i++){
     Serial.print(packedBytes[i], BIN);
     Serial.print(" ");
    }
  Serial.println("\n");

  Serial.println("VALUES AFTER PACKING: ");
  for(int i = 0; i < NumDS28AE00s*2; i++){
     Serial.print(receiveBuffer[i], BIN);
     Serial.print(" ");
    }
  Serial.println("\n");


  
  delay(5000);

}


void packTemperaturesForTransmission(int NumDS28AE00, byte receiveBuffer[160], byte* packedBytes){

  // this function takes in the number of DS28EA00s to be packed and an array (receiveBuffer) of raw 16-bit temperature values

  // length of the buffer that holds the two bytes each chip returns for its temperature reading. Therefore this must be at least 2 * the number of chips. 
  #define receiveBufferLength NumDS28AE00 * 2

  // length of the packedBytes array, which will hold the packed 12-bit temperature readings on 8-bit byte boundaries. Because we go from 16-bit temp values to 12-bit, the length is 3/4 of receiveBuffer
  #define packedBytesLength receiveBufferLength*3/4

  // packed byte array. Generating this array is the output of this script.
//  byte packedBytes[packedBytesLength];

  // temporary variable for holding each row as it's appended to packedBytes
  byte packedRow[3];
  int count = 0; 
  byte LSB1;
  byte MSB1;
  byte LSB2; 
  byte MSB2; 
  byte maskedMSB1;
  byte maskedMSB2;

  int packedCount = 0;
  for(int i = 0; i < receiveBufferLength-3; i=i+4){
    
    LSB1 = receiveBuffer[i];
    MSB1 = receiveBuffer[i+1];
    LSB2 = receiveBuffer[i+2];
    MSB2 = receiveBuffer[i+3];
  
    // add first measurement LSByte to row 1
    packedRow[0]=LSB1; 
  
    // isolate only last 4 bits of first measurement MSByte
    maskedMSB1 = MSB1 & 0b00001111; 
  
    // bit shift 4 to the right to give 4 trailing zeros
    maskedMSB1 = maskedMSB1 << 4;  
  
    // isolate last 4 bits of MSB2
    maskedMSB2 = MSB2 & 0b00001111; 
  
    // OR together to combine both into one byte
    packedRow[1] = maskedMSB1 | maskedMSB2; 
  
    //add second measurement LSB to row 3
    packedRow[2] = LSB2; 
  
    // finally, add packed row to packedBytes array
    for(int j = 0; j < 3; j++){
          packedBytes[packedCount] = packedRow[j];
          packedCount++;
        }
    }
   count++;
}
