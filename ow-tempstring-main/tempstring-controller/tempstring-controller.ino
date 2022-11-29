// this is test code for the one-wire control Arduino. It recieves commands from the SIMB3 main board. 

#include <Wire.h>                            
byte I2C_OnOff;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin(1);                                
  Wire.onReceive(BlinkLED);
}

void loop() {
delay(100);
}

void BlinkLED(int Press)
{
  I2C_OnOff = Wire.read();                      
  if (I2C_OnOff == 1)
  {
   digitalWrite(LED_BUILTIN, HIGH);                 
  }
  else if (I2C_OnOff == 0)
  {
   digitalWrite(LED_BUILTIN, LOW);                 
  } 
}
