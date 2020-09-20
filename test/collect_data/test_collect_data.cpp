#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#include "credentials.h"
#include "sensors.h"

AirSensors sensors;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  Serial.println("Initialize");
  for (size_t i = 0; i < sizeof(kSensorIds)/sizeof(size_t); i++) {
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
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi Connected :-)");
    WiFiClient client;
    HTTPClient http;
    Serial.println("Update data");
    sensors.UpdateData(client, http);
    Serial.println("List of sensors");
    sensors.PrintAllData();
  }
  delay(30 * 1000);
}
