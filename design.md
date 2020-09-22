# Some notes on the design

## Collecting data

The data is retrieved as a JSON stream from a set of sensors using the Purple Air API.

Information about the Purple Air API is available in Google Doc accesible from:
https://www.purpleair.com/sensorlist

Some information about the fiels at:
https://www2.purpleair.com/community/faq#!hc-json-object-fields

A list of sensor IDs is declared in `lib/air_sensors/sensors.cpp` (by order of
priority).

To minimize the time the board is active, and not create additional load on the
PA server, we plan to retrieve data every 3 minutes. This allows to have 3
samples to average on a period of 10 minutes, which is plenty. In addition, it
meshes well with displaying one sample per 6 minutes interval for 12 hours (=120
pixels out of our 176 pixels available in vertical format).

## Data processing

A simple mean is computed from the real time pm 2.5 concentrations collected,
together with a normalized absolute mean error to provide a rough estimate of
the quality of the samples.

Then the concentration mean is converted to an Air Quality Index using the
non-linear mapping from the
[specification](https://www.airnow.gov/sites/default/files/2018-05/aqi-technical-assistance-document-may2016.pdf).

In addition, average over different time period are simply collected from the primary sensors (the first sensor in the list that seems to have valid data).

## Graphing

**The information below was for the original intent for the air monitor.
Managing the ESP8266 flash memory will require some research. So in the mean
time, the system does not providing graphing, but just the statistics from the
sensor itself.**

Displaying a single instantaneous value is a no brainer. However, displaying a
graph of the last 12h trend opens a can of worm if you desire to also be
resilient to loss of power or user reset (during development). The historical
data needs to be persistant on the board if we want to avoid getting it from
another web service. The ESP8266 provides some flash storage and an emulated
EEPROM (also on the same flash memory). But things become more complex if we
hope to build a monitor that last a few years and not wear out the flash.

### Data space required

Each AQI sample has a value between 0 and 500 max, so 9 bits are sufficient for
a data point

Because we cannot guaranty to always record at regular intervals (for example,
the board could be shutdown for a few hours), it is necessary to record a
timestamp together with the AQI value. Otherwise, we would graph data that is
actually not continuous.

Since we plan one sample every 3 minutes, choosing a time resolution of a minute
is sufficient. By deciding to encode the timestamp on 22 bits, we can last
almost 8 full years:<br>
2 ^ 22 / ( 365.25 * 24 * 60 ) = 7.97.

If we select January 1st, 2020 for our Epoch, we should outlast the board life
(and certainly the Purple Air API).

So we have: 22 bits (timestamp) + 9 bits (AQI) + 1 bit reserve = 4 bytes.

#### Absolute minimum required

Storage for 12h at 3' interval: 12 * 20 * 4 = 960 bytes total.


### EEPROM, Flash and wear leveling

The initial plan was to use the ESP8266 EEPROM to store the samples. The EEPROM
is 4KB only, which would cause some concern about too many rewrites.

Some projects like [EEPROM_rotate](https://github.com/xoseperez/eeprom_rotate)
*increase* the EEPROM size to limit wear leveling. It turns out it is possible
simply because the ESP8266 EEPROM is only emulated on top of flash memory!

ESP8266 also supports some minimal filesystem (SPIFFS or LittleFS), however this
is not suitable to store our samples because of the minimal size of the file,
and again this is totally overkill to simply store a data serie.

*It seems difficult to obtain the exact specification from the flash memory
inside a $3 micro-controller chip. However, several forums tend to indicate that
the flash on the ESP8266 could sustain 100'000 writes.*

The official documentation about the ESP8266 filesystem provides a
[super useful flash map](
https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html).

The ESP8266 for the Huzzah comes with 4MB of flash total. 1MB is "reserved" by
default for SPIFFS. This is the flash area we will target to store our time
series. There is plenty of space to use large sample and still not worry about
wearing out the flash, but it will require a custom ring buffer on top of the
flash memory. To be continued...


