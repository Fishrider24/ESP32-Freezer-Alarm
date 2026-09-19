# ESP32-Freezer-Alarm Version 2.5 (3d printed case at bottom)
2.5 Added Spiffs file upload, so if you just want to change index.html, other files don't get wiped. Cleaned up some html and added links after uploads/updates.

2.4-Added OTA updates for App and Spiffs added password protection to Setup link.  Created a custom partitions.csv to make room for OTA updates. Also password to ESP-WIFI-MANAGER AP, both Setup button password and AP password are passed on 'const char* otaPassword = "freezeralarm";' its a good idea to change.  Username for Setup is admin. Delays the captive portal on iphone 30 seconds though..
 
2.3-Added Dark Mode and fixed some ntp bugs and Celsius alarm by.
Added a WipeAll Button to erase Wifi credentials, ip's, email sender information, and all influxdb settings.
Added influxdb 1.8, 12/24hr and timezone settings. Also Wifi Captive Portal for initial setup. Also made the bootloader button default wifi settings after holding it for 5 seconds. Better restarts if wifi turns off.
Make sure to set partition scheme(Verson 2.3 or lower, use custom for 2.4 and above): Partition Scheme: "No OTA (2MB APP/2MB SPIFFS)"
ESP32 Temp Alarm using DS18B20, wifi manager, Email alert Threshold. 

Changes From the Original Project
The original project was updated to work with newer versions of the ESP32 Arduino environment and libraries.

    Updated ESP32 Arduino Environment
    The project was updated to work with:
    - ESP32 Arduino core 3.x
    - ESP32Async ESPAsyncWebServer 3.12.0
    - ESP32Async AsyncTCP 3.5.0
    - DallasTemperature 4.x
    - OneWire 2.3.8
    - ReadyMail 0.4.x (replaces ESP-Mail-Client)
    - ESP8266 Influxdb 3.13.2
    The original code relied on older ESP32/library APIs that are no longer compatible 
    with the newer ESP32 Arduino Core.
    Added the ability to change settings without needing to upload project again.
    Now you can change Title, Gateway, IP, SSID, SSID Password, Email Sender, Email Sender Password 
    TimeZone using POSIX timezone string, 12/24hr, InfluxDB 1.8 and switch between Fahrenheit and Celsius.
    Only need to upload 3 files with spiffs. The .ino will create the rest.
    On my board I used GPIO 4 and the 3D print cutout is meant for that pin.

*****~~~~NOTES~~~~*****
AT&T doesn't do email to text anymore and Verizon is expected to go away from it. 
My work around for AT&T is to add the sender email as a VIP email on my iPhone. Then allow 
popup notifications for VIP's. Then I set the phone to check email every 30 minutes. 

To use, make sure all the libraries listed above are installed in your Arduino IDE. Put the wifimanagefreezer folder into your Arduino home folder.
Wifimanagefreezer should have the wifimanagefreezer.ino file in it and the data folder. The data folder should have the two html files and one css file. 
I made this alarm using several tutorials from https://randomnerdtutorials.com/ originally.
It has a Wifi Manager, so you dont have to hard code the wifi data into it.
Uses a DS18B20 waterproof sensor.
Has a Wifi Reconnect function.
Has an email alert if Sensortemp reading goes over setpoint, and alerts when it goes back down.
You will need to setup app passwords in gmail. https://support.google.com/mail/answer/185833?hl=en

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/board.jpeg" width="200">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/boardback.jpeg" width="200">

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/IMG_9689.png" width="200">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/IMG_9688.png" width="200">

Old copies of ESPAsyncWebServer and AsyncTCP must not be installed alongside the new versions.
Having multiple copies of AsyncTCP or ESPAsyncWebServer installed can cause compilation errors or library conflicts.

Smtp server as Gmail(lines 64,65) Change as needed.

Board settings in Arduino.&emsp;&emsp;&emsp;&emsp;&emsp;~~~***After first power-up, Connect to the ESP32 wifi***~~~

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/boardsettings.png" width="200">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/wifimanager.jpeg" width="200">

***Then the captive portal will pop up to setup your wifi credentials and the Email account that will send the alerts. Code is setup for gmail so set a app password in gmail settings. You can always change settings later from the broswer Setup button.***

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/IMG_9691.png" width="200">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/IMG_9692.png" width="200">

***Then the System will Reboot and go to the ip address that you set or defaulted 192.168.1.200***  ***!!Make sure to reconnect to your WiFi!! Type in the IP address in your browser

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/restart.png" width="200">&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/IMG_9688.png" width="200">

Set an App Password on your Gmail account!!

Made a remix of a case on Thingiverse. https://www.thingiverse.com/thing:5193607

<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/magnet.jpeg" width="200">&emsp;&emsp;<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/pageip.jpg" width="200">&emsp;&emsp;<img src="https://github.com/Fishrider24/ESP32-Freezer-Alarm/blob/main/images/wifisetup.jpg" width="200">
