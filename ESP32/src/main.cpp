#include <Arduino.h>
#include <SPI.h>

#include <BluetoothSerial.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <jquery_definition.h>
#include <update_page_definition.h>

#define SWSENSE   14
#define HALL      23
#define REL_EN    24
#define LED_BLUE  25
#define SWDRIVE   26
#define PRES_CS   29

volatile int spin = 0;

uint8_t releaseFlag = 0;

uint32_t chipID = 0;

String ssid_name;
const char *ssid;

uint8_t ledState = HIGH;

uint8_t cmd, cmdBT;
const long interval = 1000;
unsigned long currMillis, prevMillis;

BluetoothSerial SerialBT;

WebServer server(80);

// Callback for the embedded jquery.min.js page
void onJavaScript(void) {
		server.setContentLength(jquery_min_js_v3_2_1_gz_len);
		server.sendHeader(F("Content-Encoding"), F("gzip"));
    server.send_P(200, "text/javascript", jquery_min_js_v3_2_1_gz, jquery_min_js_v3_2_1_gz_len);
}

void spinCount() {
  spin++;
}

void setup() {
  for (int i = 0; i < 17; i = i + 8) {
    chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.begin(115200);
  SerialBT.begin(("ETAG_" + String(chipID)));

  Serial.println("Board starting ...");

  pinMode(SWSENSE, INPUT);
  pinMode(HALL, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL), spinCount, RISING);
  
  pinMode(REL_EN, OUTPUT);
  digitalWrite(REL_EN, LOW);

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, ledState);

  pinMode(SWDRIVE, OUTPUT);
  digitalWrite(SWDRIVE, LOW);

  // ssid may be changed if different name is desired
  ssid_name  = "Etag Comm Board " + String(chipID);
  ssid = ssid_name.c_str();
  WiFi.mode(WIFI_OFF);

  // return javascript jquery
  server.on("/jquery.min.js", HTTP_GET, onJavaScript);

  // return index page which is stored in serverIndex
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  // handling uploading firmware file
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
//        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // flashing firmware to ESP
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
//        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
      } else {
//        Update.printError(Serial);
      }
    }
  });
}

void loop() {
  if (WiFi.getMode() != WIFI_OFF) {
    server.handleClient();
    currMillis = millis();
    if (currMillis - prevMillis >= interval) {
      Serial.print("In WIFI mode\r");
      prevMillis = currMillis;
      if (ledState == HIGH) {
        ledState = LOW;
      } else {
        ledState = HIGH;
      }
      digitalWrite(LED_BLUE, ledState);
    }
  }

  if (Serial.available()) {
    cmd = Serial.read();
    SerialBT.println(cmd);
    switch (cmd) {
      case ('o'):
        Serial.println("Turning off Bluetooth and WIFI ...");
        server.stop();
        WiFi.mode(WIFI_OFF);
        SerialBT.end();
        digitalWrite(LED_BLUE, LOW);
        break;
      case ('w'):
        Serial.println("Switching to WIFI ...");
        SerialBT.end();
        digitalWrite(LED_BLUE, LOW);
        WiFi.softAP(ssid);
        server.begin();
        prevMillis = millis();
        break;
      case ('y'):
        Serial.println("Switching to Bluetooth ...");
        server.stop();
        WiFi.mode(WIFI_OFF);
        SerialBT.begin("ETAG");
        digitalWrite(LED_BLUE, HIGH);
        break;
      default:
        break;
    }
  }

  if (SerialBT.available()) {
    cmdBT = SerialBT.read();
    Serial.println(cmdBT);
    switch (cmdBT) {
      case ('o'):
        Serial.println("Turning off Bluetooth and WIFI ...");
        server.stop();
        WiFi.mode(WIFI_OFF);
        SerialBT.end();
        digitalWrite(LED_BLUE, LOW);
        break;
      case ('w'):
        Serial.println("Switching to WIFI ...");
        SerialBT.end();
        digitalWrite(LED_BLUE, LOW);
        WiFi.softAP(ssid);
        server.begin();
        prevMillis = millis();
        break;
      default:
        break;
    }
  }
}
