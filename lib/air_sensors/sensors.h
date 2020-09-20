#ifndef AAQIM_SENSORS_H
#define AAQIM_SENSORS_H

#include <stdint.h>
#include <stdlib.h>

class WiFiClient;
class HTTPClient;

static const size_t kMaxSensors = 8;

static const uint32_t kMaxReadingAgeSeconds = 12 * 60;
static const float kMaxPercentDiscrepancy = 0.06;

// Purple air sensors ID by priority.
static const size_t kSensorIds[] = {59927, 65489, 67415, 25301, 54857, 36667};

struct SensorData {
  size_t id;
  uint32_t timestamp;
  float pm_2_5_A;
  float pm_2_5_B;
  float pressure;
  int16_t age_A;
  int16_t age_B;
  int16_t temperature;
  int16_t humidity;
};

class AirSensors {
 public:
  AirSensors() : sensorsCount_(0) {
    noData_.id = 0;
    noData_.timestamp = 0;
  }

  bool AddSensor(size_t sid) {
    if (sensorsCount_ < kMaxSensors) {
      sensorIds_[sensorsCount_++] = sid;
      return true;
    }
    return false;
  }

  bool UpdateData(WiFiClient &client, HTTPClient &http);

  void PrintAllData() {
    for (size_t index = 0; index < sensorsCount_; index++) {
      PrintSensorData(sensorsData_[index]);
    }
  }

  void PrintSensorData(const SensorData &data);

  const SensorData &Data(size_t index) const {
    if (index < sensorsCount_) {
      return sensorsData_[index];
    } else {
      return noData_;
    }
  }

  size_t Count() { return sensorsCount_; }

 protected:
  size_t GetSensorIndex(size_t sid) {
    size_t index = SIZE_MAX;
    for (size_t i = 0; i < sensorsCount_; i++) {
      if (sensorIds_[i] == sid) {
        index = i;
        break;
      }
    }
    return index;
  }

  size_t ParseSensors(HTTPClient &http);

  size_t sensorsCount_;
  size_t sensorIds_[kMaxSensors];
  SensorData noData_;
  SensorData sensorsData_[kMaxSensors];
};

#endif

#if 0
      // For the sensor to be accepted as primary source,
      // it needs to:
      //   1) have been updated rencently and
      //   2) have consistency between the two embdded sensors
      if (now > 0) {
        // we have a valid timestamp to compare against
        if (abs(now - ts1) > kMaxReadingAgeSeconds ||
            abs(now - ts2) > kMaxReadingAgeSeconds) {
          return data;
        }
      } else {
        if (age1 > kMaxReadingAgeSeconds / 60 ||
            age2 > kMaxReadingAgeSeconds / 60) {
          return data;
        }
      }
      data.timestamp = ts1;
      data.pm_2_5_A = sensorsArray[0]["pm2_5_cf_1"];
      data.pm_2_5_B = sensorsArray[1]["pm2_5_cf_1"];
      if (fabs(data.pm_2_5_A - data.pm_2_5_B) /
              (data.pm_2_5_A + data.pm_2_5_B) >
          kMaxPercentDiscrepancy) {
        return data;
      }
      size_t index = 0;
      int32_t parent = sensorsArray[0]["ParentID"];
      if (parent != 0) {
        // it seems that the secondary came first
        index = 1;
      }
      data.temperature = sensorsArray[index]["temp_f"];
      data.humidity = sensorsArray[index]["humidity"];
      data.pressure = sensorsArray[index]["pressure"];
      data.id = sensorsArray[index]["ID"];

      if (data.id != 0) {
        if (primarySensorId_ == 0) {
          primarySensorId_ = data.id;
          lastTimestamp_ = data.timestamp;
        }
        sensorsData_.push_back(data);
      }
#endif
