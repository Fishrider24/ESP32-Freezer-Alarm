# ESP32-Freezer-Alarm (3d printed case at bottom)
ESP32 Temp Alarm using DS18B20, wifi manager, Email alert Threshold. 

!!Spiffs doesnt work on Arduino 2.0 or above.  I used 1.8!!

I made this alarm using several tutorials from https://randomnerdtutorials.com/.
It has a Wifi Manager, so you dont have to hard code the wifi data into it.
Uses a DS18B20 waterproof sensor.
Has a Wifi Reconnect function.
Has an email alert if Sensortemp reading goes over setpoint, and alerts when it goes back down.

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/board.jpeg" width="200">

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/boardback.jpeg" width="200">

To use, make sure all the libraries labeled below are installed in your Arduino IDE. Put the wifimanagefreezer folder into your Arduino home folder.
Wifimanagefreezer should have the wifimanagefreezer.ino file in it and the data folder. The data folder should have the nine .txt files, two html files and one css file. 

***Need to add .txt files to the Data Folder as below***
check.txt, email.txt, epass.txt, input.txt, inputcc.txt, input3.txt, ip.txt, pass.txt and ssid.txt

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/alarmwebpage.png" width="200">

Libraries Used:
ESPAsyncWebServer--This and AsyncTCP installed using DS18B20 tutorial
AsyncTCP           @ https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/ 

OneWire 2.3.6

DallasTemperature 3.9.0

ESP32 Mail Client 2.1.6

***After first power-up, Connect to the ESP32 wifi*** 

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/wifimanager.jpeg" width="200">

***Then go to IP address 192.168.4.1 to setup your wifi credentials and the Email account that will send the alerts. If using gmail set a app password in gmail settings.***

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/wifisetup.png" width="200">

***Then the System will Reboot and go to the ip address that you set or defaulted 192.168.1.200***

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/restart.png" width="200">

***!!Make sure to reconnect to your WiFi!! Type in the IP address in your browser

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/alarmwebpage.png" width="200">

***Check bottom if programming from Raspberry Pi***

Tutorials used below

ESP32 add-on Arduino IDE

Rui Santos
  Complete project details at https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/

ESP32 Wifi Manager async

Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  
Libraries Required:ESPAsyncWebServer, AsyncTCP

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/pinout.jpeg" width="200">

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

Library required: ESP32 Mail Client(Had to downoad newest version from their website and manually install using Pi.
Set an App Password on your Gmail account!!

Running ESP32 Filesystem Uploader
Make a folder called tools in the /home/pi/Arduino folder and unzip ESP32FS into the tools folder
Should look like this /home/pi/Arduino/tools/ESP32FS/
##############################

Made a remix of a case on Thingiverse. https://www.thingiverse.com/thing:5193607

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/magnet.jpeg" width="200"><img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/pageip.jpeg" width="200"><img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/wifisetup.jpeg" width="200">
