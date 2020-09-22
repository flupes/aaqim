#ifndef AAQIM_SENSORS_H
#define AAQIM_SENSORS_H

#include <stdint.h>
#include <stdlib.h>

class WiFiClient;
class HTTPClient;

const size_t kMaxSensors = 8;

// Purple air sensors ID by priority.
static const size_t kSensorIds[] = {59927, 65489, 67415, 25301, 54857, 36667};

enum PmAvgIndexes {
  TenMinutes,
  ThirtyMinutes,
  OneHour,
  SixHours,
  TwentyFourHours,
  OneWeek,
  PmAvgSize
};

struct SensorData {
  size_t id;
  uint32_t timestamp;
  float pm_2_5_A;
  float pm_2_5_B;
  float pressure;
  float averages[PmAvgIndexes::PmAvgSize];
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

  size_t Count() const { return sensorsCount_; }

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
