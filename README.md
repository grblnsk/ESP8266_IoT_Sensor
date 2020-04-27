# Internet-of-Things Sensing Device on ESP8266
In this repository you can find code for ESP8266, a low-cost Wi-Fi microchip, that reads pressure and temperature data from Bosch's BMP180 Barometric Pressure/Temperature/Altitude Sensor, and broadcasts it over WiFi network using UDP protocol.

Notes:

https://github.com/esp8266/esp8266-wiki/wiki/Toolchain

PATH=/opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin:$PATH

build:
make

flash:
sudo make ESPPORT=/dev/ttyUSB0 flash

sudo screen /dev/ttyUSB0 115200