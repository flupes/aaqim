#include "sensors.h"

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// Information about the Purple Aor API from:
// https://www.purpleair.com/sensorlist
// Look also at:
// https://www2.purpleair.com/community/faq#!hc-json-object-fields

// To test with 8 sensors collection:
// http://www.purpleair.com/json?show=59927|65489|67415|25301|54857|36667|66029|54411
// ArduinoJson Assistant says 19416 bytes

static const size_t kJsonCapacity = 20000;
static const char *kPurpleAirUrl = "http://www.purpleair.com/";
static const char *kPurpleAirRequest = "json?show=";

bool AirSensors::UpdateData(WiFiClient &client, HTTPClient &http) {
  String url = String(kPurpleAirUrl) + String(kPurpleAirRequest);
  Serial.println(sensorsCount_);
  for (size_t index = 0; index < sensorsCount_; index++) {
    // Build request to obtain all the sensors at once
    if (index > 0) {
      url += "|";
    }
    url += String(sensorIds_[index]);
  }
  Serial.print("Request URL = ");
  Serial.println(url);

  unsigned long start = millis();
  // Send request
  http.useHTTP10(true);
  http.begin(client, url);
  http.GET();
  Serial.print("Retrieve data time (ms) = ");
  Serial.println(millis()-start);
  
  return ( ParseSensors(http) > 0);
}

void AirSensors::PrintSensorData(const SensorData &data) {
  Serial.print("Sensor # ");
  Serial.println(data.id);
  Serial.print("  timestamp =   ");
  Serial.println(data.timestamp);
  Serial.print("  age_A (min) = ");
  Serial.println(data.age_A);
  Serial.print("  age_B (min) = ");
  Serial.println(data.age_B);
  Serial.print("  pm_2_5_A =    ");
  Serial.println(data.pm_2_5_A);
  Serial.print("  pm_2_5_B =    ");
  Serial.println(data.pm_2_5_B);
  Serial.print("  temperature = ");
  Serial.println(data.temperature);
  Serial.print("  humidity =    ");
  Serial.println(data.humidity);
  Serial.print("  pressure =    ");
  Serial.println(data.pressure);
}

size_t AirSensors::ParseSensors(HTTPClient& http) {
  Serial.print("Memory heap before JSON doc = ");
  Serial.println(ESP.getFreeHeap());
  DynamicJsonDocument doc(kJsonCapacity);
  Serial.print("Memory heap after JSON doc = ");
  Serial.println(ESP.getFreeHeap());

  unsigned long start = millis();
  auto err = deserializeJson(doc, http.getStream());
  if (err) {
    Serial.print(F("deserializeJson() returned "));
    Serial.println(err.c_str());
  }
  unsigned long step = millis();
  Serial.print("Deserialization (ms) = ");
  Serial.println(step-start);

  // Array of sensors
  JsonArray sensorsArray = doc["results"].as<JsonArray>();
  // Unfortunately, the array or returned sensors is not ordered
  // the same way that the request is made.

  size_t primaryCount = 0;
  for (JsonObject sensor : sensorsArray) {
    size_t index;
    size_t parent = sensor["ParentID"];
    if (parent == 0) {
      size_t sid = sensor["ID"];
      index = GetSensorIndex(sid);
      if (index < kMaxSensors) {
        sensorsData_[index].id = sid;
        sensorsData_[index].timestamp = sensor["LastSeen"];
        sensorsData_[index].pm_2_5_A = sensor["pm2_5_cf_1"];
        sensorsData_[index].age_A = sensor["AGE"];
        sensorsData_[index].temperature = sensor["temp_f"];
        sensorsData_[index].humidity = sensor["humidity"];
        sensorsData_[index].pressure = sensor["pressure"];
        primaryCount++;
      }
    } else {
      index = GetSensorIndex(parent);
      sensorsData_[index].pm_2_5_B = sensor["pm2_5_cf_1"];
      sensorsData_[index].age_B = sensor["AGE"];
    }
  }

  unsigned long stop = millis();
  Serial.print("Parsing to structure (ms) = ");
  Serial.println(stop-step);
 
  return primaryCount;
}