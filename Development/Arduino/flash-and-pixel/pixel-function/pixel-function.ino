#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);





void setup() {
  // put your setup code here, to run once:
  pixels.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  pixel(true, "GREEN");
  delay(2000);
  pixel(true, "PURPLE");
  delay(2000);
  pixel(true, "RED");
  delay(2000);
  pixel(true, "BLUE");
  delay(2000);
  pixel(false, "BLUE");
  delay(2000);
}



void pixel(bool on, char* color){

  if(color == "GREEN"){
      pixels.setPixelColor(0, pixels.Color(45,255,0));
  }
  else if(color == "BLUE"){
          pixels.setPixelColor(0, pixels.Color(0,255,255));
  }
  else if(color == "PURPLE"){
          pixels.setPixelColor(0, pixels.Color(121,0,255));
  }
  else if(color == "RED"){
          pixels.setPixelColor(0, pixels.Color(255,0,0));
  }

  if(on){
      pixels.setBrightness(10);
      pixels.show();
  }
  else{
      pixels.setBrightness(0);
      pixels.show();
  }
}