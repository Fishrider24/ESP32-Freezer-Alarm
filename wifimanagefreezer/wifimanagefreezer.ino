/*
  ESP32 Freezer Alarm - updated for current ESP32 Arduino libraries

  Original Wi-Fi Manager/web-server structure was based in part on:
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://randomnerdtutorials.com/esp32-wi-fi-manager-asyncwebserver/

  This project has been substantially modified for a freezer alarm application.

  Main library updates:
    - ESP32 Arduino core 3.x
    - ESP32Async ESPAsyncWebServer
    - ESP32Async AsyncTCP
    - DallasTemperature 4.x
    - OneWire 2.3.8
    - ReadyMail 0.4.x (replaces ESP-Mail-Client)

  The existing SPIFFS files and web pages are intentionally retained so
  existing configuration files can continue to be used.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#define ENABLE_SMTP
#include <ReadyMail.h>

// -----------------------------------------------------------------------------
// Hardware
// -----------------------------------------------------------------------------

// DS18B20 data wire is connected to GPIO 4.
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// -----------------------------------------------------------------------------
// Temperature / timing
// -----------------------------------------------------------------------------

String temperatureF = "";
String temperatureC = "";
String timenow;
String lastTemperature;
String tempUnit = "F";

unsigned long lastTemperatureRead = 0;
const unsigned long temperatureInterval = 30000UL; // 30 seconds

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -21600;       // Central Standard Time
const int daylightOffset_sec = 3600;     // Daylight Saving Time

// -----------------------------------------------------------------------------
// Email
// -----------------------------------------------------------------------------

#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "[ALERT] Temperature"

String freezername = "Freezer";
String inputMessage;       // Primary recipient
String inputMessagecc;     // CC recipient
String enableEmailChecked;
String inputMessage2;      // "true" / "false"
String inputMessage3;      // Temperature threshold
bool restartRequested = false;

// These are loaded from SPIFFS.
String emailSenderAccount;
String emailSenderPassword;

// ReadyMail SMTP client.
WiFiClientSecure ssl_client;
SMTPClient smtp(ssl_client);

// ReadyMail status callback reports SMTP progress and server responses.
bool emailSent = false;

// -----------------------------------------------------------------------------
// Web server
// -----------------------------------------------------------------------------

AsyncWebServer server(80);

const char* PARAM_INPUT_0 = "freezername";
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_GATEWAY = "gateway";
const char* PARAM_INPUT_4 = "email_input";
const char* PARAM_INPUT_5 = "enable_email_input";
const char* PARAM_INPUT_6 = "threshold_input";
const char* PARAM_INPUT_7 = "emailSender";
const char* PARAM_INPUT_8 = "emailSenderPass";
const char* PARAM_INPUT_9 = "email_inputcc";
const char* reboot = "reboot";

// -----------------------------------------------------------------------------
// Wi-Fi configuration
// -----------------------------------------------------------------------------

String ssid;
String pass;
String ip;
String emailSender;
String emailSenderPass;

const char* freezernamePath = "/freezername.txt";
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";
const char* emailSenderPath = "/email.txt";
const char* emailSenderPassPath = "/epass.txt";
const char* inputMessagePath = "/input.txt";
const char* inputMessageccPath = "/inputcc.txt";
const char* inputMessage2Path = "/check.txt";
const char* inputMessage3Path = "/input3.txt";
const char* tempUnitPath = "/tempunit.txt";

IPAddress localIP;
String gatewayIP;
IPAddress gateway;
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

unsigned long lastWiFiReconnect = 0;
const unsigned long wifiReconnectInterval = 10000UL;

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

void smtpCallback(SMTPStatus status);
bool sendEmailNotification(const String& emailMessage);

// -----------------------------------------------------------------------------
// SPIFFS helpers
// -----------------------------------------------------------------------------

void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("ERROR: SPIFFS could not be mounted.");
    return;
  }
  Serial.println("SPIFFS mounted successfully.");
}

String readFile(fs::FS &fs, const char* path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent = file.readStringUntil('\n');
  file.close();
  fileContent.trim();
  return fileContent;
}

void writeFile(fs::FS &fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }

  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }

  file.close();
}

// -----------------------------------------------------------------------------
// Temperature
// -----------------------------------------------------------------------------

bool readTemperature() {
  sensors.requestTemperatures();

  float tempC = sensors.getTempCByIndex(0);
  float tempF = sensors.getTempFByIndex(0);

  // DS18B20 returns approximately -127 C / -196.6 F when disconnected.
  if (tempC <= -126.0f || tempF <= -196.0f) {
    Serial.println("ERROR: Failed to read DS18B20 sensor.");
    temperatureC = "--";
    temperatureF = "--";
    return false;
  }

  temperatureC = String(tempC, 2);
  temperatureF = String(tempF, 2);
  lastTemperature = temperatureF;

  Serial.print("Temperature: ");
  Serial.print(temperatureF);
  Serial.println(" F");

  return true;
}

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

void printLocalTime() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo, 5000)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeStringBuff[64];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %I:%M:%S", &timeinfo);
  timenow = timeStringBuff;
  Serial.println(timeStringBuff);
}

// -----------------------------------------------------------------------------
// Web page placeholder processor
// -----------------------------------------------------------------------------

String processor(const String& var) {
  if (var == "TEMPERATUREF") {
    return temperatureF;
  }
  else if (var == "TEMPERATUREC") {
    return temperatureC;
  }
  else if (var == "TIMESTAMP") {
    return timenow;
  }
  else if (var == "EMAIL_INPUT") {
    return inputMessage;
  }
  else if (var == "EMAIL_INPUTCC") {
    return inputMessagecc;
  }
  else if (var == "ENABLE_EMAIL") {
    return enableEmailChecked;
  }
  else if (var == "THRESHOLD") {
    return inputMessage3;
  }
  else if (var == "IP_ADDRESS") {
    String currentIP = WiFi.localIP().toString();
    if (currentIP != "0.0.0.0") {
      return currentIP;
    } else {
      return "192.168.1.200";
    }
  }
  else if (var == "GATEWAY_ADDRESS") {
    if (gatewayIP.length() > 0) {
      return gatewayIP;
    } else {
      return "192.168.1.1";
    }
  }
  else if (var == "FREEZER_NAME") {
    if (freezername.length() > 0) {
      return freezername;
    } else {
      return "Freezer Alarm";
    }
  }
  else if (var == "TEMP_UNIT_F") {
    return tempUnit == "F" ? "checked" : "";
  }
  else if (var == "TEMP_UNIT_C") {
    return tempUnit == "C" ? "checked" : "";
  }
  else if (var == "TEMP_UNIT") {
    return tempUnit;
  }
  return String();
}

// -----------------------------------------------------------------------------
// Wi-Fi
// -----------------------------------------------------------------------------

bool initWiFi() {
  if (ssid.length() == 0 || ip.length() == 0) {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  // setHostname() must be called before starting Wi-Fi.
  WiFi.setHostname("ESP32-Freezer-Alarm");
  WiFi.mode(WIFI_STA);

  if (!localIP.fromString(ip)) {
    Serial.println("Invalid static IP address.");
    return false;
  }

  if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA failed to configure.");
    return false;
  }

  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.print("Connecting to WiFi");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < wifiReconnectInterval) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi.");
    return false;
  }

  Serial.print("Connected. IP address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  return true;
}

// -----------------------------------------------------------------------------
// Email using ReadyMail
// -----------------------------------------------------------------------------

void smtpCallback(SMTPStatus status) {
  // ReadyMail is asynchronous. Report its current SMTP state/response,
  // rather than waiting for a completion flag.
  if (status.progress.available) {
    Serial.printf("ReadyMail[smtp][%d] %s %d%%\n",
                  status.state,
                  status.progress.filename.c_str(),
                  status.progress.value);
  } else {
    Serial.printf("ReadyMail[smtp][%d] %s\n",
                  status.state,
                  status.text.c_str());
  }
}

bool sendEmailNotification(const String& emailMessage) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send email: WiFi is not connected.");
    return false;
  }

  if (emailSenderAccount.length() == 0 || emailSenderPassword.length() == 0) {
    Serial.println("Cannot send email: sender account/password not configured.");
    return false;
  }

  if (inputMessage.length() == 0) {
    Serial.println("Cannot send email: recipient is not configured.");
    return false;
  }

  // Gmail SMTP on port 465 uses implicit SSL/TLS.
  ssl_client.setInsecure();  // Certificate verification can be added later.

  Serial.println("Connecting to Gmail SMTP server...");

  smtp.connect(smtpServer, smtpServerPort, smtpCallback);

  if (!smtp.isConnected()) {
    Serial.println("ReadyMail: SMTP connection failed.");
    return false;
  }

  Serial.println("ReadyMail: SMTP connected.");

  smtp.authenticate(emailSenderAccount.c_str(),
                    emailSenderPassword.c_str(),
                    readymail_auth_password);

  if (!smtp.isAuthenticated()) {
    Serial.println("ReadyMail: SMTP authentication failed.");
    return false;
  }

  Serial.println("ReadyMail: SMTP authentication successful.");

  SMTPMessage message;

  message.headers.add(
      rfc822_from,
      freezername + " <" + emailSenderAccount + ">");

  message.headers.add(
      rfc822_to,
      "Freezer Alarm <" + inputMessage + ">");

  if (inputMessagecc.length() > 0) {
    message.headers.add(
        rfc822_cc,
        inputMessagecc);
  }

  message.headers.add(
      rfc822_subject,
      freezername + " - " + emailSubject);

  message.text.body(emailMessage);

  // ReadyMail recommends setting the message timestamp after NTP sync.
  time_t now = time(nullptr);
  if (now > 100000) {
    message.timestamp = now;
  }

  Serial.println("Sending email...");
  smtp.send(message);

  // ReadyMail sends asynchronously.  The current library examples call
  // smtp.send(message) without waiting for a completion flag.  Reaching this
  // point means the message was accepted by ReadyMail for sending; the
  // smtpCallback() above reports the actual SMTP progress/response.
  Serial.println("ReadyMail: email accepted for sending.");
  return true;
}

// -----------------------------------------------------------------------------
// Web server setup
// -----------------------------------------------------------------------------

void startNormalWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
  });
  server.on("/wifimanager", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/wifimanager.html", "text/html", false, processor);
  });

  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", temperatureC);
  });

  server.on("/temperaturef", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", temperatureF);
  });

  server.on("/timenow", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", timenow);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam(PARAM_INPUT_4)) {
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      writeFile(SPIFFS, inputMessagePath, inputMessage.c_str());
    }

    if (request->hasParam(PARAM_INPUT_9)) {
      inputMessagecc = request->getParam(PARAM_INPUT_9)->value();
      writeFile(SPIFFS, inputMessageccPath, inputMessagecc.c_str());
    }

    if (request->hasParam(PARAM_INPUT_5)) {
      inputMessage2 = "true";
      enableEmailChecked = "checked";
    } else {
      inputMessage2 = "false";
      enableEmailChecked = "";
    }
    writeFile(SPIFFS, inputMessage2Path, inputMessage2.c_str());

    if (request->hasParam(PARAM_INPUT_6)) {
      inputMessage3 = request->getParam(PARAM_INPUT_6)->value();
      writeFile(SPIFFS, inputMessage3Path, inputMessage3.c_str());
    }

    Serial.println("Settings updated. Restarting...");

    request->send(200, "text/plain",
              "Settings saved. Restarting...");
    delay(1000);
    request->redirect("/");
    restartRequested = true;
  });
  server.on("/wifimanager", HTTP_POST, [](AsyncWebServerRequest* request) {
    int params = request->params();

    for (int i = 0; i < params; i++) {
      const AsyncWebParameter* p = request->getParam(i);

      if (!p->isPost()) {
        continue;
      }

      if (p->name() == PARAM_INPUT_0 && p->value().length() > 0) {
        freezername = p->value();
        writeFile(SPIFFS, freezernamePath, freezername.c_str());
        Serial.print("Freezer Name set to: ");
        Serial.println(freezername);
      }
      else if (p->name() == PARAM_INPUT_1 && p->value().length() > 0) {
        ssid = p->value();
        writeFile(SPIFFS, ssidPath, ssid.c_str());
        Serial.println("SSID updated.");
      }
      else if (p->name() == PARAM_INPUT_2 && p->value().length() > 0) {
        pass = p->value();
        writeFile(SPIFFS, passPath, pass.c_str());
        Serial.println("WiFi password updated.");
      }
      else if (p->name() == PARAM_INPUT_3 && p->value().length() > 0) {
        ip = p->value();
        writeFile(SPIFFS, ipPath, ip.c_str());
        Serial.print("IP address set to: ");
        Serial.println(ip);
      }
      else if (p->name() == PARAM_INPUT_7 && p->value().length() > 0) {
        emailSender = p->value();
        writeFile(SPIFFS, emailSenderPath, emailSender.c_str());
        Serial.println("Email sender updated.");
      }
      else if (p->name() == PARAM_INPUT_8 && p->value().length() > 0) {
        emailSenderPass = p->value();
        writeFile(SPIFFS, emailSenderPassPath, emailSenderPass.c_str());
        Serial.println("Email sender password updated.");
      }
      else if (p->name() == "temp_unit") {
        tempUnit = p->value();

        if (tempUnit != "F" && tempUnit != "C") {
          tempUnit = "F";
        }

        writeFile(SPIFFS, tempUnitPath, tempUnit.c_str());

        Serial.print("Temperature unit set to: ");
        Serial.println(tempUnit);
      }
      else if (p->name() == "reboot") {
        Serial.println("Restart requested.");
        request->send(200, "text/plain", "Restarting...");
        delay(1000);
        request->redirect("/");
        restartRequested = true;
        return;
      }
      else if (p->name() == PARAM_INPUT_GATEWAY && p->value().length() > 0) {
        gatewayIP = p->value();
        if (gateway.fromString(gatewayIP)) {
          writeFile(SPIFFS, gatewayPath, gatewayIP.c_str());
          Serial.print("Gateway set to: ");
          Serial.println(gatewayIP);
        } else {
          gateway.fromString("192.168.1.1");
          gatewayIP = "192.168.1.1";
        }
      }
    }

    request->send(200, "text/plain", "Settings saved. Restarting...");
    delay(1000);
    request->redirect("/");
    restartRequested = true;
  });

  server.begin();
  Serial.println("Web server started.");
}

void startWiFiManagerServer() {
  Serial.println("Setting up WiFi Manager access point.");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP-WIFI-MANAGER");

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/wifimanager.html", "text/html", false, processor);
  });

  server.serveStatic("/", SPIFFS, "/");

  server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {
    int params = request->params();

    for (int i = 0; i < params; i++) {
      const AsyncWebParameter* p = request->getParam(i);

      if (!p->isPost()) {
        continue;
      }
      if (p->name() == PARAM_INPUT_0) {
        freezername = p->value();
        Serial.print("Freezer Name set to: ");
        Serial.println(freezername);
        writeFile(SPIFFS, freezernamePath, freezername.c_str());
      }
      if (p->name() == PARAM_INPUT_1 && p->value().length() > 0) {
        ssid = p->value();
        Serial.print("SSID set to: ");
        Serial.println(ssid);
        writeFile(SPIFFS, ssidPath, ssid.c_str());
      }
      else if (p->name() == PARAM_INPUT_2 && p->value().length() > 0) {
        pass = p->value();
        Serial.println("WiFi password updated.");
        writeFile(SPIFFS, passPath, pass.c_str());
      }
      else if (p->name() == PARAM_INPUT_3 && p->value().length() > 0) {
        ip = p->value();
        Serial.print("IP address set to: ");
        Serial.println(ip);
        writeFile(SPIFFS, ipPath, ip.c_str());
      }
      else if (p->name() == PARAM_INPUT_7 && p->value().length() > 0) {
        emailSender = p->value();
        Serial.print("Email sender set to: ");
        Serial.println(emailSender);
        writeFile(SPIFFS, emailSenderPath, emailSender.c_str());
      }
      else if (p->name() == PARAM_INPUT_8 && p->value().length() > 0) {
        emailSenderPass = p->value();
        Serial.println("Email sender password updated.");
        writeFile(SPIFFS, emailSenderPassPath, emailSenderPass.c_str());
      }
      else if (p->name() == "temp_unit") {
        tempUnit = p->value();

        if (tempUnit != "F" && tempUnit != "C") {
          tempUnit = "F";
        }

        writeFile(SPIFFS, tempUnitPath, tempUnit.c_str());

        Serial.print("Temperature unit set to: ");
        Serial.println(tempUnit);
      }
      else if (p->name() == "reboot") {
        request->send(200, "text/plain", "Restarting...");
        delay(1000);
        request->redirect("/");
        restartRequested = true;
      }
      else if (p->name() == PARAM_INPUT_GATEWAY && p->value().length() > 0) {
        gatewayIP = p->value();
        if (gateway.fromString(gatewayIP)) {
          writeFile(SPIFFS, gatewayPath, gatewayIP.c_str());
          Serial.print("Gateway set to: ");
          Serial.println(gatewayIP);
        } else {
          gateway.fromString("192.168.1.1");
          gatewayIP = "192.168.1.1";
        }
      }
    }

    request->send(200, "text/plain",
                  "Done. ESP will restart, connect to your router and go to IP address: " + ip);
    restartRequested = true;
  });
  server.on("/wifimanager", HTTP_POST, [](AsyncWebServerRequest* request) {
    int params = request->params();

    for (int i = 0; i < params; i++) {
      const AsyncWebParameter* p = request->getParam(i);

      if (!p->isPost()) {
        continue;
      }

      if (p->name() == PARAM_INPUT_0 && p->value().length() > 0) {
        freezername = p->value();
        writeFile(SPIFFS, freezernamePath, freezername.c_str());
      }
      else if (p->name() == PARAM_INPUT_1 && p->value().length() > 0) {
        ssid = p->value();
        writeFile(SPIFFS, ssidPath, ssid.c_str());
      }
      else if (p->name() == PARAM_INPUT_2 && p->value().length() > 0) {
        pass = p->value();
        writeFile(SPIFFS, passPath, pass.c_str());
      }
      else if (p->name() == PARAM_INPUT_3 && p->value().length() > 0) {
        ip = p->value();
        writeFile(SPIFFS, ipPath, ip.c_str());
      }
      else if (p->name() == PARAM_INPUT_7 && p->value().length() > 0) {
        emailSender = p->value();
        writeFile(SPIFFS, emailSenderPath, emailSender.c_str());
      }
      else if (p->name() == PARAM_INPUT_8 && p->value().length() > 0) {
        emailSenderPass = p->value();
        writeFile(SPIFFS, emailSenderPassPath, emailSenderPass.c_str());
      }
      else if (p->name() == "temp_unit") {
        tempUnit = p->value();

        if (tempUnit != "F" && tempUnit != "C") {
          tempUnit = "F";
        }

        writeFile(SPIFFS, tempUnitPath, tempUnit.c_str());

        Serial.print("Temperature unit set to: ");
        Serial.println(tempUnit);
      }
      else if (p->name() == "reboot") {
        request->send(200, "text/plain", "Restarting...");
        delay(1000);
        request->redirect("/");
        restartRequested = true;
      }
      else if (p->name() == PARAM_INPUT_GATEWAY && p->value().length() > 0) {
        gatewayIP = p->value();
        if (gateway.fromString(gatewayIP)) {
          writeFile(SPIFFS, gatewayPath, gatewayIP.c_str());
          Serial.print("Gateway set to: ");
          Serial.println(gatewayIP);
        } else {
          gateway.fromString("192.168.1.1");
          gatewayIP = "192.168.1.1";
        }
      }
    }

    request->send(200, "text/plain", "Settings saved. Restarting...");
    delay(1000);
    request->redirect("/");
    restartRequested = true;
  });
  server.begin();
  Serial.println("WiFi Manager web server started.");
}

// -----------------------------------------------------------------------------
// Create Files
// -----------------------------------------------------------------------------

void createDefaultFiles() {
  if (!SPIFFS.exists("/freezername.txt"))
    writeFile(SPIFFS, "/freezername.txt", "Freezer");

  if (!SPIFFS.exists("/ssid.txt"))
    writeFile(SPIFFS, "/ssid.txt", "");

  if (!SPIFFS.exists("/pass.txt"))
    writeFile(SPIFFS, "/pass.txt", "");

  if (!SPIFFS.exists("/ip.txt"))
    writeFile(SPIFFS, "/ip.txt", "192.168.1.200");

  if (!SPIFFS.exists("/gateway.txt"))
    writeFile(SPIFFS, "/gateway.txt", "192.168.1.1");

  if (!SPIFFS.exists("/email.txt"))
    writeFile(SPIFFS, "/email.txt", "");

  if (!SPIFFS.exists("/epass.txt"))
    writeFile(SPIFFS, "/epass.txt", "");

  if (!SPIFFS.exists("/input.txt"))
    writeFile(SPIFFS, "/input.txt", "");

  if (!SPIFFS.exists("/inputcc.txt"))
    writeFile(SPIFFS, "/inputcc.txt", "");

  if (!SPIFFS.exists("/check.txt"))
    writeFile(SPIFFS, "/check.txt", "false");

  if (!SPIFFS.exists("/input3.txt"))
    writeFile(SPIFFS, "/input3.txt", "");

  if (!SPIFFS.exists("/tempunit.txt"))
    writeFile(SPIFFS, "/tempunit.txt", "F");
}

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32 Freezer Alarm - Updated Version");
#ifdef ESP_ARDUINO_VERSION_STR
  Serial.print("ESP32 Arduino core: ");
  Serial.println(ESP_ARDUINO_VERSION_STR);
#endif
  Serial.println("========================================");

  initSPIFFS();

  createDefaultFiles();

  // Load values saved in SPIFFS.
  freezername = readFile(SPIFFS, freezernamePath);
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gatewayIP = readFile(SPIFFS, gatewayPath);
  if (!gateway.fromString(gatewayIP)) {
    gateway.fromString("192.168.1.1");
    gatewayIP = "192.168.1.1";
  }
  emailSenderAccount = readFile(SPIFFS, emailSenderPath);
  emailSenderPassword = readFile(SPIFFS, emailSenderPassPath);
  inputMessage = readFile(SPIFFS, inputMessagePath);
  inputMessagecc = readFile(SPIFFS, inputMessageccPath);
  inputMessage2 = readFile(SPIFFS, inputMessage2Path);
  inputMessage3 = readFile(SPIFFS, inputMessage3Path);
  tempUnit = readFile(SPIFFS, tempUnitPath);

  if (tempUnit != "F" && tempUnit != "C") {
    tempUnit = "F";
  }

  // Do not print passwords or other credentials to Serial.
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Static IP: ");
  Serial.println(ip);
  Serial.print("Sender email: ");
  Serial.println(emailSenderAccount);
  Serial.print("Recipient: ");
  Serial.println(inputMessage);
  Serial.print("CC: ");
  Serial.println(inputMessagecc);
  Serial.print("Email enabled: ");
  Serial.println(inputMessage2);
  Serial.print("Temperature threshold: ");
  Serial.println(inputMessage3);

  if (inputMessage2 == "true") {
    enableEmailChecked = "checked";
  } else {
    inputMessage2 = "false";
    enableEmailChecked = "";
  }

  // Start the DS18B20 library once during setup.
  sensors.begin();
  sensors.setResolution(12);

  readTemperature();

  if (initWiFi()) {
    startNormalWebServer();
  } else {
    startWiFiManagerServer();
  }
}

// -----------------------------------------------------------------------------
// Main loop
// -----------------------------------------------------------------------------

void loop() {
  unsigned long currentMillis = millis();

  // Read temperature and check alarm every 30 seconds.
  if (currentMillis - lastTemperatureRead >= temperatureInterval || lastTemperatureRead == 0) {
    lastTemperatureRead = currentMillis;

    if (WiFi.status() == WL_CONNECTED) {
      if (readTemperature()) {
        printLocalTime();

        float temperature = sensors.getTempFByIndex(0);
        float threshold = inputMessage3.toFloat();

        // Check if temperature is above threshold and an alert has not already
        // been sent.
        if (temperature > threshold && inputMessage2 == "true" && !emailSent) {
          String emailMessage = String("Temperature above threshold. Current temperature: ") +
                                String(temperature, 2) + " F";

          if (sendEmailNotification(emailMessage)) {
            Serial.println(emailMessage);
            emailSent = true;
          } else {
            Serial.println("Email failed to send.");
          }
        }
        // Reset the alarm after temperature falls at least 1 F below the
        // configured threshold.
        else if (temperature < (threshold - 1.0f) &&
                 inputMessage2 == "true" && emailSent) {
          String emailMessage = String("Temperature below threshold. Current temperature: ") +
                                String(temperature, 2) + " F";

          if (sendEmailNotification(emailMessage)) {
            Serial.println(emailMessage);
            emailSent = false;
          } else {
            Serial.println("Email failed to send.");
          }
        }
      }
    }
  }

  // If WiFi goes down, periodically try to reconnect.
  if (WiFi.getMode() == WIFI_STA &&
      WiFi.status() != WL_CONNECTED &&
      currentMillis - lastWiFiReconnect >= wifiReconnectInterval) {

    lastWiFiReconnect = currentMillis;
    Serial.println("Reconnecting to WiFi...");

    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), pass.c_str());
  }
  if (restartRequested) {
    delay(1500);
    ESP.restart();
  }
  // Keep the loop responsive.
  delay(10);
}
