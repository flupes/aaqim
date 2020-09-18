# aaqim = Arduino AQI Monitor

## Why

The air quality has been a real concern in the West from August 2020.
[Purple Air](https://www.purpleair.com/map) and [Air Now](https://gispub.epa.gov/airnow/) maps are great tools to evaluate if you can dare point your nose outside the confine of your shelter or not. However, I wanted a solution that provides the current state of pollution in a glimpse, without relying on logging on a computer.

Ideally, you would setup your own sensor and create your DIY display to monitor the pollution in real time. However, due to high demand, air quality sensors are out of stock everywhere!

The next best option is then to use the data already available. There are more than 15 Purple Air stations in a 500m diameter circle around my house, so it is possible to get data localized enough to be relevant. The next requirement for me was to create a low power solution: I would prefer to not contribute more to the global warming just for my personal convinience. So a typical trivial solution relying on a Raspberry Pi single board computer is not considered here.

So this project relies on micro-controller with WiFi capabilities. This project is built using an ESP8266 (because I had this old board at hand). And the display is based on a E-Paper panel that does not consume any current if not updated. Since air quality is a slow evolving parameter, this leads to a very low power consumption solution.

## Bill of Material

Not by design, but un-unsed parts from previous projects:
  - ESP8266 (Adafruit Huzzah)[https://www.adafruit.com/product/2821]
  - (2.7in E-paper Dislay)[https://www.embeddedartists.com/products/2-7-inch-e-paper-display/]
  - LiPo battery (need to find one).


