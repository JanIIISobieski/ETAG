#include <Arduino.h>

#include <SPI.h>
#include <MS58xx.h>

#include <BluetoothSerial.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <jquery_definition.h>
#include <update_page_definition.h>

#define SWSENSE   12
#define HALL      15
#define REL_EN    2
#define LED_BLUE  0
#define SWDRIVE   4
#define PRES_CS   5

volatile int32_t spin = 0;

uint8_t releaseFlag = LOW;

uint32_t chipID = 0;

String ssid_name;
const char *ssid;

uint8_t ledState = HIGH;

uint8_t cmd, cmdBT;
const long interval = 1000;
unsigned long currMillis, prevMillis;

MS58xx pressSens;

BluetoothSerial SerialBT;

WebServer server(80);

// Callback for the embedded jquery.min.js page
void onJavaScript(void) {
		server.setContentLength(jquery_min_js_v3_2_1_gz_len);
		server.sendHeader(F("Content-Encoding"), F("gzip"));
    server.send_P(200, "text/javascript", jquery_min_js_v3_2_1_gz, jquery_min_js_v3_2_1_gz_len);
}

void IRAM_ATTR spinCount() {
  spin ++;
//  Serial.println("spin interrupt");
}

void setup() {
  for (int i = 0; i < 17; i = i + 8) {
    chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.begin(115200);
  SerialBT.begin(("ETAG_" + String(chipID)));

  //Serial.print("LBoard starting ...\n");

  pinMode(SWSENSE, INPUT);
  pinMode(HALL, INPUT);
  attachInterrupt(HALL, spinCount, RISING);
  
  pinMode(REL_EN, OUTPUT);
  digitalWrite(REL_EN, releaseFlag);

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, ledState);

  pinMode(SWDRIVE, OUTPUT);
  digitalWrite(SWDRIVE, LOW);

  pinMode(PRES_CS, OUTPUT);
  digitalWrite(PRES_CS, HIGH);

  pressSens.init(PRES_CS);
//  Serial.println("PSENS: " + String(pressSens.PSENS));
//  Serial.println("POFF: " + String(pressSens.POFF));
//  Serial.println("TCSENS: " + String(pressSens.TCSENS));
//  Serial.println("TCOFF: " + String(pressSens.TCOFF));
//  Serial.println("TREF: " + String(pressSens.TREF));
//  Serial.println("TEMPSENS: " + String(pressSens.TEMPSENS));

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
      //Serial.print("LIn WIFI mode\n");
//      Serial.print((String) "Hall: " + digitalRead(HALL) + "\r");
      prevMillis = currMillis;
      ledState = !ledState;
//      if (ledState == HIGH) {
//        ledState = LOW;
//      } else {
//        ledState = HIGH;
//      }
      digitalWrite(LED_BLUE, ledState);
    }
  }

  if (Serial.available()) {
    cmd = Serial.read();
    SerialBT.write(cmd);
    switch (cmd) {
      case ('o'):
        //Serial.print("LTurning off Bluetooth and WIFI ...\n");
        server.stop();
        WiFi.mode(WIFI_OFF);
        SerialBT.end();
        digitalWrite(LED_BLUE, LOW);
        break;
      case ('p'):
        //Serial.print("LPressue and Temperature reading:\n");
        pressSens.calc_press_temp();
        //Serial.print((String) "LPressure: " + pressSens.pressure_mbar + " mbar\n");
        //Serial.print((String) "LTemperature: " + pressSens.temperature + " C\n");
        break;
      case ('v'):
        //Serial.print("LCurrent spin count: ");
        //Serial.print((String) spin + "\n");
        break;
      case ('w'):
        //Serial.print("LSwitching to WIFI ...\n");
        SerialBT.end();
        digitalWrite(LED_BLUE, LOW);
        WiFi.softAP(ssid);
        server.begin();
        prevMillis = millis();
        break;
      case ('y'):
        //Serial.print("LSwitching to Bluetooth ...\n");
        server.stop();
        WiFi.mode(WIFI_OFF);
        SerialBT.begin(("ETAG_" + String(chipID)));
        digitalWrite(LED_BLUE, HIGH);
        break;
      default:
        break;
    }
  }

  if (SerialBT.available()) {
    cmdBT = SerialBT.read();
    Serial.write(cmdBT);
    switch (cmdBT) {
      case ('o'):
        //Serial.print("LTurning off Bluetooth and WIFI ...\n");
        server.stop();
        WiFi.mode(WIFI_OFF);
        SerialBT.end();
        digitalWrite(LED_BLUE, LOW);
        break;
      case ('p'):
        //Serial.print("LPressue and Temperature reading:\n");
        pressSens.calc_press_temp();
        //Serial.print((String) "LPressure: " + pressSens.pressure_mbar + " mbar\n");
        //Serial.print((String) "LTemperature: " + pressSens.temperature + " C\n");
        break;
      case ('v'):
        //Serial.print("Current spin count: ");
        //Serial.print((String) spin + "\n");
        break;
      case ('w'):
        //Serial.print("Switching to WIFI ...\n");
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
