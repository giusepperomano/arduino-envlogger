# Arduino Mega2560 Temperature and Humidity Logger
Simple "cloud enabled" room temperature and humidity logger built to play with
the DHT22 sensor and the ESP8266 based ESP-01 Wi-Fi module. The data is
logged to dweet.io and can be displayed via freeboard.io.

I am using an Arduino Mega2560 board for this project (it's possible to
use the ESP8266 as a standalone microcontroller). The configuration variables
(Wi-Fi SSID, password, etc) are stored in the Arduino EEPROM
