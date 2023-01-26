#include <Express_M0_Flash.h>

Express_M0_Flash onBoardFlash;

void setup() {

  while(!Serial){}
  Serial.print("Serial mounted");
}

void loop() {

  onBoardFlash.initialize();
  onBoardFlash.format();
  delay(5000);
}
