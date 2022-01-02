# ESP32-Freezer-Alarm
ESP32 Temp Alarm using DS18B20, wifi manager, Email alert Threshold
I made this alarm using several tutorials from https://randomnerdtutorials.com/.
It has a Wifi Manager, so you dont have to hard code the wifi data into it.
Uses a DS18B20 waterproof sensor.
Has a Wifi Reconnect function.
Has an email alert if Sensortemp reading goes over setpoint, and alerts when it goes back down.

Libraries Used:
ESPAsyncWebServer--This and AsyncTCP installed using DS18B20 tutorial
AsyncTCP           @ https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/ 

OneWire 2.3.6

DallasTemperature 3.9.0

ESP32 Mail Client 2.1.6

***Check bottom if programming from Raspberry Pi***

Tutorials used below

ESP32 add-on Arduino IDE

Rui Santos
  Complete project details at https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/

ESP32 Wifi Manager async

Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  
Libraries Required:ESPAsyncWebServer, AsyncTCP

***Wire Using this tutorial***

**ESP32 DS18B20 Temp Sensor, Webserver**

Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/ 

Libraries required: OneWire, DallasTemperature

**Wifi Reconnect**

Rui Santos
  Complete project details at https://RandomNerdTutorials.com/solved-reconnect-esp32-to-wifi/

**ESP32 Spiffs**

Rui Santos
  Complete project details at https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/ 
  
Running ESP32 Filesystem Uploader
Unzip ESP32FS into Arduino/tools folder

**ESP32 Email Alert Temp Threshold**

Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-email-alert-temperature-threshold/ 
  
******If Using Raspberry Pi to program ESP32*******

Library required: ESP32 Mail Client(Had to downoad newest version from there wesite and manually install using Pi.
Enable Less secure app access on Gmail account

Running ESP32 Filesystem Uploader
Make a folder called tools in the /home/pi/Arduino folder and unzip ESP32FS into the tools folder
##############################
