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

#define COLORED 1
#define UNCOLORED 0

Epd epd;
GFXcanvas1 *canvas[2];

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
  if ( str[0] == '1' ) {
    offset = h/8;
  }
  canvas[bufferId]->setCursor((EPD_WIDTH - w) / 2 - offset, line);
  canvas[bufferId]->print(str);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  Serial.println("Init e-Paper...");
  if (epd.Init() != 0) {
    Serial.println("e-Paper init failed");
    return;
  }
  epd.ClearFrame();

  canvas[0] = new GFXcanvas1(EPD_WIDTH, EPD_HEIGHT);
  canvas[1] = new GFXcanvas1(EPD_WIDTH, EPD_HEIGHT);

  for (uint8_t i = 0; i < 2; i++) {
    canvas[i]->fillScreen(UNCOLORED);
    canvas[i]->setTextColor(COLORED);
    canvas[i]->setTextSize(1);
    canvas[i]->setTextWrap(false);
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

    AirSample sample;
    size_t nbSamples = ComputeStats(sensors, sample);
    if (nbSamples > 0) {
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
      if ( aqi > 100 ) {
        canvas[1]->fillRoundRect(7, 36, EPD_WIDTH-2*7, 68, 8, COLORED);
        canvas[1]->fillRoundRect(10, 39, EPD_WIDTH-2*10, 62, 4, UNCOLORED);
      }
      String aqiValue(aqi);
      CenterText(&ClearSans_Bold48pt7b, aqiValue.c_str(), 96);

      String aqiName(AqiNames[static_cast<int>(sample.Level())]);
      CenterText(&ClearSans_Medium18pt7b, aqiName.c_str(), 132);

      String count = String("# ") + sample.SamplesCount() + " sensors";
      CenterText(&ClearSans_Medium12pt7b, count.c_str(), 160);

      float nmae = sample.NmaeValue();
      if ( nmae > 0.1 ) {
        canvas[1]->fillRoundRect(34, 164, EPD_WIDTH-2*34, 24, 6, COLORED);
        canvas[1]->fillRoundRect(36, 166, EPD_WIDTH-2*36, 20, 4, UNCOLORED);
      }
      String stats = String("err=") +
                     String(nmae * 100.0f, 0) + String("%");
      CenterText(&ClearSans_Medium12pt7b, stats.c_str(), 182);

    } else {
      Serial.println("No valid air sample retrieved :-(");
    }

  } else {
    // not connected, too bad :-(
    Serial.println("Could not connected to WiFi :-(");
  }

  epd.TransmitPartial(canvas[0]->getBuffer(), canvas[1]->getBuffer(), 0, 0,
                      EPD_WIDTH, EPD_HEIGHT);
  epd.DisplayFrame();

  Serial.println("Put display to sleep");
  epd.Sleep();
  delay(500);

  Serial.println("Now go to sleep for 4 minutes");
  ESP.deepSleep(4 * 60 * 1E6);
}

void loop() {}

#else
// Something trivial to avoid pio errors
int main(void) { return 0; }
#endif
