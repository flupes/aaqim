#ifndef AAQIM_SENSORS_H
#define AAQIM_SENSORS_H

#if defined(ARDUINO)

#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include <stdlib.h>
#include <stdint.h>
#include <vector>

static const size_t kMaxSensors = 8;
static const size_t kMaxReadings = 16;

static const size_t kJsonCapacity = 4096;

static const uint32_t kMaxReadingAgeSeconds = 12 * 60;
static const float kMaxPercentDiscrepancy = 0.06;

// Information about the Purple Aor API from:
// https://www.purpleair.com/sensorlist
// Look also at:
// https://www2.purpleair.com/community/faq#!hc-json-object-fields
static const char *kPurpleAirUrl = "http://www.purpleair.com/";
static const char *kPurpleAirRequest = "json?show=";

// Purple air sensors ID by priority.
static const size_t kSensorIds[] = {59927, 65489, 67415, 25301, 54857, 36667};

struct SensorCombo {
  size_t id;
  uint32_t timestamp;
  float pm_2_5_A;
  float pm_2_5_B;
  float pressure;
  int16_t temperature;
  int16_t humidity;
};

class AirSensors {
 public:
  AirSensors() : doc_(kJsonCapacity), primarySensorId_(0) {
    http_.useHTTP10(true);
  }

  bool AddSensor(size_t id) {
    if (sensorIds_.size() < kMaxSensors) {
      sensorIds_.push_back(id);
      return true;
    }
    return false;
  }

  bool UpdateData(WiFiClient &client, uint32_t now) {
    primarySensorId_ = 0;
    sensorsData_.clear();
    for (size_t id : sensorIds_) {
      // Build request
      String url = String(kPurpleAirUrl) + String(kPurpleAirRequest) + String(id);
      // Send request
      http_.begin(client, url);
      http_.GET();
      auto err = deserializeJson(doc_, http_.getStream());
      if (err) {
        Serial.print(F("deserializeJson() returned "));
        Serial.println(err.c_str());
        break;
      }

      SensorCombo data = ParseSensor(now);
      if ( data.id != 0 ) {
        if ( primarySensorId_ == 0 ) {
          primarySensorId_ = data.id;
          lastTimestamp_ = data.timestamp;
        }
        sensorsData_.push_back(data);
      }
    }
    return (sensorsData_.size() > 0);
  }

  void PrintData() {
    for (SensorCombo data : sensorsData_) {
      Serial.print("Sensor # ");
      Serial.println(data.id);
      Serial.print("  timestamp =   ");
      Serial.println(data.timestamp);
      Serial.print("  pm_2_5_A =    " );
      Serial.println(data.pm_2_5_A);
      Serial.print("  pm_2_5_B =    " );
      Serial.println(data.pm_2_5_B);
      Serial.print("  temperature = ");
      Serial.println(data.temperature);
      Serial.print("  humidity =    ");
      Serial.println(data.humidity);
      Serial.print("  pressure =    ");
      Serial.println(data.pressure);
    }
  }

 protected:
  // Return the sensor data or 0 if sensor data is not valid
  SensorCombo ParseSensor(uint32_t now) {
    SensorCombo data;
    data.id = 0;
    // For the sensor to be accepted as primary source,
    // it needs to:
    //   1) have been updated rencently and
    //   2) have consistenncy between the two embdded sensors
    JsonArray sensorsArray = doc_["results"].as<JsonArray>();
    if (sensorsArray.size() < 2) {
      return data;
    }
    uint32_t ts1 = sensorsArray[0]["LastSeen"];
    uint32_t ts2 = sensorsArray[1]["LastSeen"];
    if (abs(now - ts1) > kMaxReadingAgeSeconds ||
        abs(now - ts2) > kMaxReadingAgeSeconds) {
      return data;
    }
    data.timestamp = ts1;
    data.pm_2_5_A = sensorsArray[0]["pm2_5_cf_1"];
    data.pm_2_5_B = sensorsArray[1]["pm2_5_cf_1"];
    if (fabs(data.pm_2_5_A - data.pm_2_5_B) / (data.pm_2_5_A + data.pm_2_5_B) > kMaxPercentDiscrepancy) {
      return data;
    }
    size_t index = 0;
    int32_t parent = sensorsArray[0]["ParentID"];
    if ( parent != 0 ) {
      // it seems that the secondary came first
      index = 1;
    }
    data.temperature = sensorsArray[index]["temp_f"];
    data.humidity = sensorsArray[index]["humidity"];
    data.pressure = sensorsArray[index]["pressure"];
    data.id = sensorsArray[index]["ID"];
    return data;
  }

  HTTPClient http_;
  DynamicJsonDocument doc_;
  size_t primarySensorId_;
  uint32_t lastTimestamp_;
  std::vector<size_t> sensorIds_;
  std::vector<SensorCombo> sensorsData_;
};

#endif

#endif
