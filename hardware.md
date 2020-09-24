# Some notes on the hardware

[Huzzah Pinout](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/pinouts)

[EPD Specs](https://www.waveshare.com/product/displays/e-paper/epaper-2/2.7inch-e-paper-hat-b.htm)

## Pin allocation

It turns out that the Huzzah has a very limited number of GPIO available: barely
enough to support this EPD!

Pins on the breakout but not available:
  - TX (#1)
  - RX (#3)

### EPD Control

| EPD # | EPD  Func. | Wire Color | Huzzah Pin         |
|------:|:----------:|:----------:|--------------------|
|     1 |    VCC     |    Red     | 3V                 |
|     2 |    GND     |   Black    | GND                |
|     3 |    DIN     |    Blue    | MOSI (#13)         |
|     4 |    CLK     |   Yellow   | SCK (#14)          |
|     5 |     CS     |   Orange   | #0 (also Red LED)  |
|     6 |     DC     |   Green    | #2 (also Blue LED) |
|     7 |    RST     |   White    | #4                 |
|     8 |    BUSY    |   Purple   | #5                 |

So the blue LED will remain ON while from the begining of the display buffer transfer to the EPD until the board goes to sleep.

### Other

Wake up from sleep:
  - #16 connected to RST

Battery Voltage:
  - Single Analog Input: ADC (1.0V max!)

Remains open:
  - #12 = MISO (not sure if available since we are using hardware SPI)
  - #15: with pullup resistor, used to detect boot-mode (yeah, one left!)


## PMS5003 Sensor

This project does not use a physical sensor, but we still need to understand the
Purple Air sensor properties to treat the data correctly. Purple Air sensors
basically combine *two* PMS5003 (and one BME280) in one package, allowing to
obtain more reliable measurements.

Purple Air does not declare the formulat to declare a 100% accuracy on a sensor
reading. From the
[PM5003 User Manual](https://cdn-shop.adafruit.com/product-files/3686/plantower-pms5003-manual_v2-3.pdf)
we some information about an individual sensor:

| Parameter           | Values                |
|---------------------|-----------------------|
| Maximum Consistency | ±10% @ 100~500μg/m³   |
| Error               | ±10μg/m³ @ 0~100μg/m³ |

So it seems that depending the particle concentration, we should use different
measure to declare that the two sensors are consistent.