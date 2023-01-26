#include <Wire.h>
#include "SIMB3_Onewire_Controller.h"

#define NUMDS28EA00s 80
#define OW_CONTROLLER_I2C_ADDRESS 11
#define NUBC_5V_ENABLE 5

String readString;
byte command[2];
byte packedTempValues[NUMDS28EA00s * 2 * 3 / 4];
int error;

void SIMB3_Onewire_Controller::reset()
{
  // reset the controller by toggling the power
  // 6 second delay needed to wait for controller to power back up

  digitalWrite(NUBC_5V_ENABLE, LOW);
  delay(1000);
  digitalWrite(NUBC_5V_ENABLE, HIGH);
  delay(6000);
}

bool SIMB3_Onewire_Controller::readTopString()
{
  Serial.println("Fetching top (upper) Temperature String values from controller...");
  command[0] = 1;
  command[1] = 0;
  Wire.beginTransmission(OW_CONTROLLER_I2C_ADDRESS);
  Wire.write(command, 2);
  error = Wire.endTransmission();
  Serial.print("--> Controller responded with error code: ");
  Serial.println(error);
  Serial.println("");
  return (true);
}

bool SIMB3_Onewire_Controller::readBottomString()
{
  Serial.println("Fetching bottom (lower) temperature string values from controller...");
  command[0] = 2;
  command[1] = 0;
  Wire.beginTransmission(OW_CONTROLLER_I2C_ADDRESS);
  Wire.write(command, 2);
  error = Wire.endTransmission();
  Serial.print("--> Controller responded with error code: ");
  Serial.println(error);
  Serial.println("");
  return (true);
}

bool SIMB3_Onewire_Controller::readAirTemperature()
{
  Serial.println("Fetching air temperature values from controller...");
  command[0] = 3;
  command[1] = 0; // packing index variable declaration not needed for air temperature, but we'll define it anyways
  Wire.beginTransmission(OW_CONTROLLER_I2C_ADDRESS);
  Wire.write(command, 2);
  error = Wire.endTransmission();
  Serial.print("--> Controller responded with error code: ");
  Serial.println(error);
  Serial.println("");
  return (true);
}

byte *SIMB3_Onewire_Controller::requestAirTempBytes(byte *airTempBuffer)
{
  int k = 0;
  Wire.requestFrom(OW_CONTROLLER_I2C_ADDRESS, 2);
  while (Wire.available())
  {
    char c = Wire.read();
    airTempBuffer[k] = c;
    k++;
  }
}

byte *SIMB3_Onewire_Controller::requestTemperatureStringBytes(int tempstring, byte *unpackedTempBuffer)
{

  int k = 0;
  for (int i = 0; i < 5; i++)
  {
    Wire.requestFrom(OW_CONTROLLER_I2C_ADDRESS, 32); // request 32 bytes at a time
    while (Wire.available())
    {
      char c = Wire.read();
      unpackedTempBuffer[k] = c;
      k++;
    }
  }

  return (unpackedTempBuffer);
}

void SIMB3_Onewire_Controller::printTemperatureString(int tempstring, byte *tempStringBuffer)
{

  tempstring == 1 ? Serial.println(F("Top String")) : Serial.println(F("Bottom String"));

  int k = 1;
  float ow_dtc_value;
  for (int i = 0; i < NUMDS28EA00s * 2; i = i + 2)
  {
    ow_dtc_value = decodeTemperatureBytes(tempStringBuffer[i], tempStringBuffer[i + 1]);
    Serial.print(k);
    Serial.print(": ");
    Serial.print(ow_dtc_value);
    Serial.print(" ");
    if (i % 8 == 0 & i != 0)
    {
      Serial.println("");
    }
    k++;
  }
  Serial.println("");
}

void SIMB3_Onewire_Controller::printAirTemperature(byte *airTempBuffer)
{

  Serial.print("Air Temperature (DEG C): ");
  float air_temp = decodeTemperatureBytes(airTempBuffer[0], airTempBuffer[1]);
  Serial.print(air_temp);
  Serial.println(" ");
}

float SIMB3_Onewire_Controller::decodeTemperatureBytes(byte LSB, byte MSB)
{

  int bitArray[16];

  int k = 0;
  for (int i = 7; i >= 0; i = i - 1)
  {
    int bits = bitRead(MSB, i);
    bitArray[k] = bits;
    k++;
  }

  for (int i = 7; i >= 0; i = i - 1)
  {
    int bits = bitRead(LSB, i);
    bitArray[k] = bits;
    k++;
  }

  float temperature = (bitArray[5] * pow(2, 6) + bitArray[6] * pow(2, 5) + bitArray[7] * pow(2, 4) + bitArray[8] * pow(2, 3) + bitArray[9] * pow(2, 2) + bitArray[10] * pow(2, 1) + bitArray[11] * pow(2, 0) + bitArray[12] * pow(2, -1) + bitArray[13] * pow(2, -2) + bitArray[14] * pow(2, -3) + bitArray[15] * pow(2, -4));
  return (temperature);
}

byte *SIMB3_Onewire_Controller::packTemperaturesForTransmission(int NumDS28AE00, byte *unpackedBytes, byte *packedBytes)
{

// this function takes in the number of DS28EA00s to be packed and an array (receiveBuffer) of raw 16-bit temperature values

// length of the buffer that holds the two bytes each chip returns for its temperature reading. Therefore this must be at least 2 * the number of chips.
#define receiveBufferLength NumDS28AE00 * 2

// length of the packedBytes array, which will hold the packed 12-bit temperature readings on 8-bit byte boundaries. Because we go from 16-bit temp values to 12-bit, the length is 3/4 of receiveBuffer
#define packedBytesLength receiveBufferLength * 3 / 4

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
  for (int i = 0; i < receiveBufferLength; i = i + 4)
  {

    LSB1 = unpackedBytes[i];
    MSB1 = unpackedBytes[i + 1];
    LSB2 = unpackedBytes[i + 2];
    MSB2 = unpackedBytes[i + 3];

    // add first measurement LSByte to row 1
    packedRow[0] = LSB1;

    // isolate only last 4 bits of first measurement MSByte
    maskedMSB1 = MSB1 & 0b00001111;

    // bit shift 4 to the right to give 4 trailing zeros
    maskedMSB1 = maskedMSB1 << 4;

    // isolate last 4 bits of MSB2
    maskedMSB2 = MSB2 & 0b00001111;

    // OR together to combine both into one byte
    packedRow[1] = maskedMSB1 | maskedMSB2;

    // add second measurement LSB to row 3
    packedRow[2] = LSB2;

    for (int j = 0; j < 3; j++)
    {
      packedBytes[packedCount] = packedRow[j];
      packedCount++;
    }
  }
  count++;

  return (packedBytes);
}