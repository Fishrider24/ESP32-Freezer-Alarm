
/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  Complete project details at https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
  Complete project details at https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/ 
  Complete project details at https://RandomNerdTutorials.com/solved-reconnect-esp32-to-wifi/
  Complete project details at https://RandomNerdTutorials.com/esp32-email-alert-temperature-threshold/
  Complete project details at https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/  

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/


#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#include "ESP32_MailClient.h"


// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
String temperatureF = "";
String temperatureC = "";
String timeupdate;
String timeStringBuff;
String timenow;
// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -21600;
const int   daylightOffset_sec = 3600;

// To send Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
// YOU MUST ENABLE less secure app option https://myaccount.google.com/lesssecureapps?pli=1
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "[ALERT] ESP32 Temperature"

// Default Recipient Email Address
String inputMessage;
String inputMessagecc;
//String enableEmailChecked = "checked";
//String inputMessage2 = "true";
String enableEmailChecked;
String inputMessage2;

// Default Threshold Temperature Value
String inputMessage3;
String lastTemperature;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

SMTPData smtpData;

bool emailSent = false;

const char* PARAM_INPUT_4 = "email_input";
const char* PARAM_INPUT_5 = "enable_email_input";
const char* PARAM_INPUT_6 = "threshold_input";

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_7 = "emailSender";
const char* PARAM_INPUT_8 = "emailSenderPass";
const char* PARAM_INPUT_9 = "email_inputcc";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String emailSender;
String emailSenderPass;
String emailSenderAccount;
String emailSenderPassword;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* emailSenderPath = "/email.txt";
const char* emailSenderPassPath = "/epass.txt";
const char* inputMessagePath = "/input.txt";
const char* inputMessageccPath = "/inputcc.txt";
const char* inputMessage2Path = "/check.txt";
const char* inputMessage3Path = "/input3.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

/*String readDSTemperatureC() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  if(tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC); 
  }
  return String(tempC);
}*/

String readDSTemperatureF() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempF = sensors.getTempFByIndex(0);

  if(int(tempF) == -196){
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Temperature Fahrenheit: ");
    Serial.println(tempF);
  }
  return String(tempF);
}

// Replaces placeholder with DS18B20 values
String processor(const String& var){
  //Serial.println(var);
/*  if(var == "TEMPERATUREC"){
    return temperatureC;
  }*/
  if(var == "TEMPERATUREF"){
    return temperatureF;
  }
  else if(var == "TIMESTAMP"){
    return timenow;
  }
  else if(var == "EMAIL_INPUT"){
    return inputMessage;
  }
  else if(var == "EMAIL_INPUTCC"){
    return inputMessagecc;
  }
  else if(var == "ENABLE_EMAIL"){
    return enableEmailChecked;
  }
  else if(var == "THRESHOLD"){
    return inputMessage3;
  }
  return String();
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %I:%M:%S", &timeinfo);
  timenow = timeStringBuff;
  Serial.println(timeStringBuff);
  String asString(timeStringBuff);
}

// Initialize WiFi
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }
 // Start up the DS18B20 library
  sensors.begin();

  
/*  temperatureC = readDSTemperatureC();*/
  temperatureF = readDSTemperatureF();
  
  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());

  if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;

// Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  initSPIFFS();
// Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  
  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  emailSenderAccount = readFile(SPIFFS, emailSenderPath);
  emailSenderPassword = readFile(SPIFFS, emailSenderPassPath);
  inputMessage = readFile(SPIFFS, inputMessagePath);
  inputMessagecc = readFile(SPIFFS, inputMessageccPath);
  inputMessage2 = readFile(SPIFFS, inputMessage2Path);
  inputMessage3 = readFile(SPIFFS, inputMessage3Path);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(emailSenderAccount);
  Serial.println(emailSenderPassword);
  Serial.println(inputMessage);
  Serial.println(inputMessagecc);
  Serial.println(inputMessage2);
  Serial.println(inputMessage3);

  if(inputMessage2 == "true"){
      enableEmailChecked = "checked";
  }
  if(inputMessage2 == "false"){
      enableEmailChecked = "";
  }
  if(initWiFi()) {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });
    server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperatureC.c_str());
    });
    server.on("/temperaturef", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperatureF.c_str());
    });
    server.on("/timenow", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", timenow.c_str());
    });
    // Receive an HTTP GET request at <ESP_IP>/get?email_input=<inputMessage>&enable_email_input=<inputMessage2>&threshold_input=<inputMessage3>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET email_input value on <ESP_IP>/get?email_input=<inputMessage>
      if (request->hasParam(PARAM_INPUT_4)) {
       inputMessage = request->getParam(PARAM_INPUT_4)->value();
       Serial.println(inputMessage);
         // Write file to save value
         writeFile(SPIFFS, inputMessagePath, inputMessage.c_str());
         // GET email_input value on <ESP_IP>/get?email_input=<inputMessagecc>
       if (request->hasParam(PARAM_INPUT_9)) {
         inputMessagecc = request->getParam(PARAM_INPUT_9)->value();
         Serial.println(inputMessagecc);
         // Write file to save value
         writeFile(SPIFFS, inputMessageccPath, inputMessagecc.c_str());
       // GET enable_email_input value on <ESP_IP>/get?enable_email_input=<inputMessage2>
       if (request->hasParam(PARAM_INPUT_5)) {
         inputMessage2 = request->getParam(PARAM_INPUT_5)->value();
         Serial.println(inputMessage2);
         inputMessage2 = "true";
         enableEmailChecked = "checked";
          // Write file to save value
         writeFile(SPIFFS, inputMessage2Path, inputMessage2.c_str());
        }
       else {
         inputMessage2 = "false";
         enableEmailChecked = "";
         writeFile(SPIFFS, inputMessage2Path, inputMessage2.c_str());
       }
        // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage3>
       if (request->hasParam(PARAM_INPUT_6)) {
         inputMessage3 = request->getParam(PARAM_INPUT_6)->value();
         Serial.println(inputMessage3);
         // Write file to save value
         writeFile(SPIFFS, inputMessage3Path, inputMessage3.c_str());
       }
      }
      }
     else {
       inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    Serial.println(inputMessagecc);
    Serial.println(inputMessage2);
    Serial.println(inputMessage3);
    request->send(200, "text/html", "HTTP GET request sent to your ESP. Controller will restart.<br><a href=\"/\">Return to Home Page</a>");
    delay(2000);
    ESP.restart();
  });
    server.begin();
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", SPIFFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          // HTTP POST emailSender value
          if (p->name() == PARAM_INPUT_7) {
            emailSender = p->value().c_str();
            Serial.print("Email sender set to: ");
            Serial.println(emailSender);
            // Write file to save value
            writeFile(SPIFFS, emailSenderPath, emailSender.c_str());
          }
          // HTTP POST emailSenderPass value
          if (p->name() == PARAM_INPUT_8) {
            emailSenderPass = p->value().c_str();
            Serial.print("Email Sender Password set to: ");
            Serial.println(emailSenderPass);
            // Write file to save value
            writeFile(SPIFFS, emailSenderPassPath, emailSenderPass.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(2000);
      ESP.restart();
    });
    server.begin();
  }
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
/*    temperatureC = readDSTemperatureC();*/
    temperatureF = readDSTemperatureF();
    printLocalTime();
    lastTime = millis();
    float temperature = sensors.getTempFByIndex(0);
    lastTemperature = String(temperature);
        // Check if temperature is above threshold and if it needs to send the Email alert
    if(temperature > inputMessage3.toFloat() && inputMessage2 == "true" && !emailSent){
      String emailMessage = String("Temperature above threshold. Current temperature: ") + 
                            String(temperature) + String("F");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }    
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if((temperature < (inputMessage3.toFloat()-1)) && inputMessage2 == "true" && emailSent) {
      String emailMessage = String("Temperature below threshold. Current temperature: ") + 
                            String(temperature) + String(" F");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent = false;
      }
      else {
        Serial.println("Email failed to send");
      }
    }
  }
  unsigned long currentMillis = millis();
// if WiFi is down, try reconnecting
if ((WiFi.status() != WL_CONNECTED) && (currentMillis - lastTime >=timerDelay)) {
  Serial.print(millis());
  Serial.println("Reconnecting to WiFi...");
  WiFi.disconnect();
  WiFi.reconnect();
  lastTime = currentMillis;
}
}

bool sendEmailNotification(String emailMessage){
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be 
  // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
  //smtpData.setSTARTTLS(true);

  // Set the sender name and Email
  smtpData.setSender("ESP32", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);
  
  if(inputMessagecc != ""){
      smtpData.addCC(inputMessagecc);
  }
  smtpData.setSendCallback(sendCallback);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
  // Clear all data from Email object to free memory
  smtpData.empty();
  return true;
}

// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }  
}
