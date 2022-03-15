#include <Arduino.h>

#include <BluetoothSerial.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <jquery_definition.h>
#include <update_page_definition.h>

// ssid may be changed if different name is desired
const char *ssid = "Etag Comm Board";

const int outputLED = 25;
int ledState = HIGH;

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

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ETAG");

  Serial.println("Board starting ...");

  pinMode(outputLED, OUTPUT);
  digitalWrite(outputLED, ledState);

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
      ledState = !ledState;
      digitalWrite(outputLED, ledState);
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
        digitalWrite(outputLED, LOW);
        break;
      case ('w'):
        Serial.println("Switching to WIFI ...");
        SerialBT.end();
        digitalWrite(outputLED, LOW);
        WiFi.softAP(ssid);
        server.begin();
        prevMillis = millis();
        break;
      case ('y'):
        Serial.println("Switching to Bluetooth ...");
        server.stop();
        WiFi.mode(WIFI_OFF);
        SerialBT.begin("ETAG");
        digitalWrite(outputLED, HIGH);
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
        digitalWrite(outputLED, LOW);
        break;
      case ('w'):
        Serial.println("Switching to WIFI ...");
        SerialBT.end();
        digitalWrite(outputLED, LOW);
        WiFi.softAP(ssid);
        server.begin();
        prevMillis = millis();
        break;
      default:
        break;
    }
  }
}
