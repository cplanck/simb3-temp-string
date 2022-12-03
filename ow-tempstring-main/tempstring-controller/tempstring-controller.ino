// this is test code for the one-wire control Arduino. It recieves commands from the SIMB3 main board. 

// modify this code to send data back to master 

#include <Wire.h>       
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire  ds(12);  // on pin 12 (a 4.7K (ish) resistor is necessary, it wont work without it)

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&ds);
                     
byte command;
#define ANSWERSIZE 50
String answer = "Hello";

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin(1);                                
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  sensors.begin();
}

void loop() {
delay(100);
}


// called when master sends data
void receiveEvent(int numBytes){
  
  command = Wire.read();

  if (command == 1){
   digitalWrite(LED_BUILTIN, HIGH);                 
  }
  
  else if (command == 0){
   digitalWrite(LED_BUILTIN, LOW);                 
  } 

  else if (command == 2){
    answer = "Hi";
  }
  else if (command == 3){
    answer = "it me";
  }

  else if (command == 0x55){
    answer = "general string";
  }
}

// called when master asks for data
void requestEvent() {
 
  // Setup byte variable in the correct size
  byte response[ANSWERSIZE];
  
  // Format answer as array
  for (byte i=0;i<ANSWERSIZE;i++) {
    response[i] = (byte)answer.charAt(i);
  }

  sensors.requestTemperatures();
  int temp = sensors.getTempCByIndex(0)*100;

    Wire.write(temp,sizeof(temp));

  
  // Send response back to Master
//  Wire.write(response,sizeof(response));
}
