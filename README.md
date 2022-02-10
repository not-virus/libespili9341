# libespili9341
An ILI9341 display driver library for the esp8266 microcontroller written purely in C

**NOTE**: This library is still in development and is not ready to use. I've put it up here for the curious.

## About
This library can be used to interface an ESP8266 with the Adafruit ILI9341 LCD display. Due to the esp8266's
limited GPIO pin count, this will be done over the SPI bus with the help of the
[libespezspi](https://github.com/not-virus/libespezspi) library. This means the display's refresh rate will
be quite slow, but a fast refresh rate is currently not a requirement of this project.
