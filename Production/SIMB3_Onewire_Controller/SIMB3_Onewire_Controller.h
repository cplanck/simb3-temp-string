/*
  SIMB3_Onewiretempstring.h - Library for interacting with the Cryosphere Innovation SIMB3 One-Wire Temperature String
  Created by Cameron Planck, 15 December 2022.
*/

#include "Arduino.h"

class SIMB3_Onewire_Controller
{
public:
  bool readTopString();
  bool readBottomString();
  bool readAirTemperature();
  void reset();
  byte *requestTemperatureStringBytes(int tempstring, byte *unpackedTempBuffer);
  byte *requestAirTempBytes(byte *airTempBuffer);
  byte *packTemperaturesForTransmission(int NumDS28AE00, byte *receiveBuffer, byte *packedBytes);
  void copyTempstringBuffer(int tempstring, byte *TempStringBuffer);
  void printTemperatureString(int tempstring, byte *tempStringBuffer);
  void printAirTemperature(byte *airTempBuffer);
  void printTemperatureString(int tempstring);
  float decodeTemperatureBytes(byte LSB, byte MSB);
};