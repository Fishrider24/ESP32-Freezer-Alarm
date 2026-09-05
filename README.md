# ESP32-Freezer-Alarm Version 2!! (3d printed case at bottom)
ESP32 Temp Alarm using DS18B20, wifi manager, Email alert Threshold. 

Changes From the Original Project
The original project was updated to work with newer versions of the ESP32 Arduino environment and libraries.

    Updated ESP32 Arduino Environment
    The project was updated to work with:
    Arduino IDE 2.3.x
    ESP32 Arduino Core 3.x
    Current ESP32Async libraries
    Current DallasTemperature / OneWire libraries
    ReadyMail for SMTP email
    The original code relied on older ESP32/library APIs that are no longer compatible with the newer ESP32 Arduino Core.
    Added the ability to change settings without needing to upload project again.
    Now you can change Title, Gateway, IP, SSID, SSID Password, Email Sender, Email Sender Password and switch between Fahrenheit and Celsius.
    Only need to upload 3 files with spiffs. The .ino will create the rest.

I made this alarm using several tutorials from https://randomnerdtutorials.com/ originally.
It has a Wifi Manager, so you dont have to hard code the wifi data into it.
Uses a DS18B20 waterproof sensor.
Has a Wifi Reconnect function.
Has an email alert if Sensortemp reading goes over setpoint, and alerts when it goes back down.
You will need to setup app passwords in gmail. https://support.google.com/mail/answer/185833?hl=en

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/board.jpeg" width="200">

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/boardback.jpeg" width="200">

To use, make sure all the libraries labeled below are installed in your Arduino IDE. Put the wifimanagefreezer folder into your Arduino home folder.
Wifimanagefreezer should have the wifimanagefreezer.ino file in it and the data folder. The data folder should have the two html files and one css file. 

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/webpage.png" width="200">

Main library updates:
    - ESP32 Arduino core 3.x
    - ESP32Async ESPAsyncWebServer
    - ESP32Async AsyncTCP
    - DallasTemperature 4.x
    - OneWire 2.3.8
    - ReadyMail 0.4.x (replaces ESP-Mail-Client)
Old copies of ESPAsyncWebServer and AsyncTCP must not be installed alongside the new versions.
Having multiple copies of AsyncTCP or ESPAsyncWebServer installed can cause compilation errors or library conflicts.

Smtp server as Gmail(lines 64,65) and gmtOffset(lines 56-58) for timezone are still hard coded.  Change as needed

Board settings in Arduino.

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/boardsettings.png" width="200">

***After first power-up, Connect to the ESP32 wifi*** 

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/wifimanager.jpeg" width="200">

***Then go to IP address 192.168.4.1 to setup your wifi credentials and the Email account that will send the alerts. If using gmail set a app password in gmail settings.***

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/manager.png" width="200">

***Then the System will Reboot and go to the ip address that you set or defaulted 192.168.1.200***

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/restart.png" width="200">

***!!Make sure to reconnect to your WiFi!! Type in the IP address in your browser

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/webpage.png" width="200">

Set an App Password on your Gmail account!!

Made a remix of a case on Thingiverse. https://www.thingiverse.com/thing:5193607

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/magnet.jpeg" width="200"><img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/pageip.jpg" width="200"><img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/wifisetup.jpg" width="200">
