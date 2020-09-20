#include <Arduino.h>

#include "credentials.h"
#include "sensors.h"

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  AirSensors sensors;
  for (size_t i = 0; i < 3; i++) {
    sensors.AddSensor(kSensorIds[i]);
  }

  Serial.print("Looking for wifi ");
  unsigned long start = millis();
  bool timeout = false;
  WiFi.begin(SSID, PASS);
  while (!timeout && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 45 * 1000) {
      timeout = true;
    }
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    sensors.UpdateData(client, 0);
    sensors.PrintData();
  }
}

void loop() {}