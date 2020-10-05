#if defined(ARDUINO)

#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <time.h>

#include "Fonts/ClearSans-Bold-48pt7b.h"
#include "Fonts/ClearSans-Medium-12pt7b.h"
#include "Fonts/ClearSans-Medium-18pt7b.h"
#include "air_sample.h"
#include "analyze.h"
#include "cfaqi.h"
#include "credentials.h"
#include "epd2in7b.h"
#include "sensors.h"
#include "display_samples.h"

#define COLORED 1
#define UNCOLORED 0

Epd epd;
// This is ugly. TODO: encapsulate these 2 canvas in a proper class
GFXcanvas1 *canvas[2];

EspFlash gFlash;
FlashAirDataSamples gFlashSamples(gFlash, 0xA000);

// Use the AD converted of the ESP8266 to read the chip supply
// voltage (instean of the analog input pin)
ADC_MODE(ADC_VCC);

const time_t kTimeZoneOffsetSeconds = -7 * 3600;

enum class ColorBuffer : uint8_t { Black = 0, Red = 1 };

void CenterText(const GFXfont *font, const char *str, uint16_t line,
                ColorBuffer color = ColorBuffer::Black) {
  int16_t x, y;
  uint16_t w, h;
  uint8_t bufferId = static_cast<uint8_t>(color);
  canvas[bufferId]->setFont(font);
  canvas[bufferId]->getTextBounds(str, 0, 100, &x, &y, &w, &h);
  int16_t offset = 0;
  if (str[0] == '1') {
    offset = h / 8;
  }
  canvas[bufferId]->setCursor((EPD_WIDTH - w) / 2 - offset, line);
  canvas[bufferId]->print(str);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  Serial.print("Memory heap entering setup() = ");
  size_t startHeap = ESP.getFreeHeap();
  Serial.println(startHeap);
  Serial.print("Fragmentation = ");
  Serial.println(ESP.getHeapFragmentation());

  gFlashSamples.Begin();
  printf("nb of sectors in use : %d\n", gFlashSamples.SectorsInUse());
  printf("first addr of reserved : 0x%08X\n", gFlashSamples.FlashStorageStart());
  printf("end of reserved flash  : 0x%08X\n", gFlashSamples.FlashStorageEnd());
  printf("nominal number of samples : %d\n", gFlashSamples.NominalCapacity());
  printf("current number of samples stored : %d\n", gFlashSamples.NumberOfSamples());

// Wiped flash in 4986 ms
// nb of sectors in use : 160
// last addr of reserved : 3801088
// nominal number of samples : 40704
// current number of samples stored : 0

// UT flash start : 3801088

  Serial.println("Init e-Paper...");
  if (epd.Init() != 0) {
    Serial.println("e-Paper init failed");
    return;
  }
  epd.ClearFrame();

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
    Serial.println();
    Serial.println("WiFi Connected :-)");
    WiFiClient client;
    HTTPClient http;

    AirSensors sensors;
    for (size_t i = 0; i < sizeof(kSensorIds) / sizeof(size_t); i++) {
      sensors.AddSensor(kSensorIds[i]);
    }
    Serial.println("Update data");
    sensors.UpdateData(client, http);
    Serial.println("List of sensors");
    sensors.PrintAllData();

    // Disconnect
    http.end();

    Serial.print("Memory heap before Canvas = ");
    Serial.println(ESP.getFreeHeap());
    canvas[0] = new GFXcanvas1(EPD_WIDTH, EPD_HEIGHT);
    canvas[1] = new GFXcanvas1(EPD_WIDTH, EPD_HEIGHT);
    Serial.print("Memory heap after Canvas = ");
    Serial.println(ESP.getFreeHeap());

    for (uint8_t i = 0; i < 2; i++) {
      canvas[i]->fillScreen(UNCOLORED);
      canvas[i]->setTextColor(COLORED);
      canvas[i]->setTextSize(1);
      canvas[i]->setTextWrap(false);
    }

    AirSample sample;
    int32_t primaryIndex;
    size_t nbSamples = ComputeStats(sensors, sample, primaryIndex);
#if 0
    // Photo Op only :-)
    sample.Set(sample.Seconds(), 0.0, 98.1, 0.0, 0.0, 0, 0, sample.SamplesCount(), 18.4);
    //sample.Set(sample.Seconds(), 0.0, 1.4, 0.0, 0.0, 0, 0, sample.SamplesCount(), 0.2);
#endif

    if (nbSamples > 0) {
      // Store permanently sample to flash
      AirSampleData compacted;
      sample.ToData(compacted);
      gFlashSamples.StoreSample(compacted);

      time_t seconds = sample.Seconds() + kTimeZoneOffsetSeconds;
      tm *local = gmtime(&seconds);
      char date[16];
      strftime(date, 16, "%b %d, %Y", local);
      char stamp[16];
      strftime(stamp, 16, "%H:%M:%S", local);
      Serial.print(date);
      Serial.print(" - ");
      Serial.println(stamp);
      char datetime[16];
      strftime(datetime, 16, "%b %d, %H:%M", local);

      CenterText(&ClearSans_Medium12pt7b, datetime, 24);

      int16_t aqi = sample.AqiPm_2_5();
      Serial.print("AQI = ");
      Serial.print(aqi);
      Serial.print(" --> ");
      Serial.println(AqiNames[static_cast<int>(sample.Level())]);
      if (aqi > 100) {
        canvas[1]->fillRoundRect(6, 35, EPD_WIDTH - 2 * 6, 70, 8, COLORED);
        canvas[1]->fillRoundRect(10, 39, EPD_WIDTH - 2 * 10, 62, 4, UNCOLORED);
      }
      String aqiValue(aqi);
      CenterText(&ClearSans_Bold48pt7b, aqiValue.c_str(), 96);

      String aqiName(AqiNames[static_cast<int>(sample.Level())]);
      CenterText(&ClearSans_Medium18pt7b, aqiName.c_str(), 132);

      char msg[16];
      float mae = sample.MaeValue();
      if (mae > 15.0) {
        canvas[1]->fillRoundRect(28, 140, EPD_WIDTH - 2 * 28, 24, 6, COLORED);
        canvas[1]->fillRoundRect(30, 142, EPD_WIDTH - 2 * 30, 20, 4, UNCOLORED);
      }
      sprintf(msg, "MAE = %.1f", sample.MaeValue());
      CenterText(&ClearSans_Medium12pt7b, msg, 158);

      sprintf(msg, "%d sensors (#%d)", sample.SamplesCount(), primaryIndex + 1);
      CenterText(&ClearSans_Medium12pt7b, msg, 184);

      SensorData data = sensors.Data(primaryIndex);
      int16_t value30m, value1h, value6h, value24h;
      AqiLevel level;
      pm25_to_aqi(data.averages[static_cast<int>(PmAvgIndexes::ThirtyMinutes)],
                  value30m, level);
      pm25_to_aqi(data.averages[static_cast<int>(PmAvgIndexes::OneHour)],
                  value1h, level);
      pm25_to_aqi(data.averages[static_cast<int>(PmAvgIndexes::SixHours)],
                  value6h, level);
      pm25_to_aqi(
          data.averages[static_cast<int>(PmAvgIndexes::TwentyFourHours)],
          value24h, level);
      sprintf(msg, "avg. 30min = %d", value30m);
      CenterText(&ClearSans_Medium12pt7b, msg, 208);

      sprintf(msg, "1h=%d / 6h=%d", value1h, value6h);
      CenterText(&ClearSans_Medium12pt7b, msg, 228);

      // sprintf(msg, "avg. 24h = %d", value24h);
      // CenterText(&ClearSans_Medium12pt7b, msg, 248);

      uint16_t vcc = ESP.getVcc();
      Serial.print("VCC = ");
      Serial.println(vcc);
      sprintf(msg, "vcc = %d mV", vcc);
      CenterText(&ClearSans_Medium12pt7b, msg, 248);

    } else {
      Serial.println("No valid air sample retrieved :-/");
      CenterText(&ClearSans_Medium18pt7b, "No Data :-/", 132);
    }

  } else {
    // not connected, too bad :-(
    Serial.println("Could not connected to WiFi :-(");
    CenterText(&ClearSans_Medium18pt7b, "No WiFi :-(", 132);
  }

  epd.TransmitPartial(canvas[0]->getBuffer(), canvas[1]->getBuffer(), 0, 0,
                      EPD_WIDTH, EPD_HEIGHT);
  epd.DisplayFrame();

  Serial.println("Put display to sleep");
  epd.Sleep();
  delay(500);

  delete canvas[0];
  delete canvas[1];

  Serial.print("Memory heap before going to sleep = ");
  size_t sleepHeap = ESP.getFreeHeap();
  Serial.println(sleepHeap);
  Serial.print("Fragmentation = ");
  Serial.println(ESP.getHeapFragmentation());
  Serial.print("Memory potentially leaked = ");
  Serial.println(startHeap - sleepHeap);

  Serial.println("Now go to sleep for 5 minutes");
  ESP.deepSleep(5 * 60 * 1E6);
}

void loop() {}

#else
// Something trivial to avoid pio errors
int main(void) { return 0; }
#endif
