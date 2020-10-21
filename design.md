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
PA server, the program retrieves data every 5 minutes.

## Data processing

A simple mean is computed from the real time pm 2.5 concentrations collected,
together with a mean absolute error (MAE) to provide a rough estimate of
the quality of the samples.

Then the concentration mean is converted to an Air Quality Index using the
non-linear mapping from the
[specification](https://www.airnow.gov/sites/default/files/2018-05/aqi-technical-assistance-document-may2016.pdf).

In addition, average over different time period are simply collected from the
primary sensors (the first sensor in the list that seems to have valid data).

## Graphing

Displaying a single instantaneous value is a no brainer. However, displaying a
graph of the last 24h trend opens a can of worms. Keeping the board powered at
all time and the data serie in RAM is not an option since the graph would be
lost at any power outage or user reset (during testing for example). So the
historical data needs to be persistant on the board if we want to avoid getting
it from another web service. Persistant historical data also allows to go in
deep sleep between AQI refresh.

### Data space required

Each AQI sample has a value between 0 and 500 max, so 9 bits are sufficient for
a data point But things become more complex if we
hope to build a monitor that last a few years and not wear out 

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
wearing out the flash. However this required to implement a custom ring buffer
on top of the flash memory.

### Final historical data solution

Using the almost the 2MB of available flash allow to be more generous in term of
what we can store. The compacted data structure finally occupies 16 bytes and
the system declares a ring buffer of 40'960 samples. This is equivalent of 143
days of historical data that is stored on flash :-)


## Power consumption

Power consumption has been measured with two different approaches:
1) average draw when the board in on and updating the screen (~20s @ 80mA)
2) mini mAh meter over 60 samples

The result is ~0.5mAh per sample collected and graph refreshed. The power
consumption during deep sleep is negligeable compared to the display refresh.
This leads to less than 150mAh for a 24h period. This has been verified with the
tiny 500mAh (old) battery connected to the Huzzah that last more than 36h.

The project is not designed to be purely battery powered, however, you can at
anytime disconnect your the monitor from its USB charger and located it
somewhere else without worring about a power source for a while.