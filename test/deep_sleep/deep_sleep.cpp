#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000); // not sure why, maybe not mess with the D0 pin
  pinMode(0, OUTPUT);

  // Show awake state
  for (int i=0; i<8; i++) {
    digitalWrite(0, LOW);
    delay(200);
    digitalWrite(0, HIGH);
    delay(400);
  }  
  // Deep sleep mode for 12 seconds, the ESP8266 wakes up by itself when GPIO 16 
  Serial.println("Going to sleep");
  ESP.deepSleep(12E6); 
  }

void loop() {
}