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

#include <elapsedMillis.h>

//#define DEBUG_OUTPUT

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

void enableRelease(bool val) {
  digitalWrite(REL_EN, !val);
}

void enableSaltwaterSensor(bool val) {
  digitalWrite(SWDRIVE, val);
}

void serialWriteSaltwaterSensor() {
  Serial.write(analogRead(SWSENSE));
  #ifdef DEBUG_OUTPUT
    Serial.print((String) "SWSENSE: " + analogRead(SWSENSE) + "\n");
  #endif
}

void turnOffComms() {
  #ifdef DEBUG_OUTPUT
    Serial.print("LTurning off Bluetooth and WIFI ...\n");
  #endif
  server.stop();
  WiFi.mode(WIFI_OFF);
  SerialBT.end();
  digitalWrite(LED_BLUE, LOW);
}

void serialWritePressureTemperature() {
    if (pressSens.data_ready) {
      Serial.write(1);
      Serial.write((uint8_t*)(&pressSens.pressure_mbar), sizeof(pressSens.pressure_mbar));
      Serial.write((uint8_t*)(&pressSens.temperature), sizeof(pressSens.temperature));
      #ifdef DEBUG_OUTPUT
        Serial.print((String) "LPressure: " + pressSens.pressure_mbar + " mbar\n");
        Serial.print((String) "LTemperature: " + pressSens.temperature + " C\n");
      #endif
      pressSens.data_ready = false;
    } else {
      Serial.write(0);
    }
}

void resetPressure() {
  pressSens.init(PRES_CS);
  #ifdef DEBUG_OUTPUT
    Serial.print("LPressure sensor reset\n");
  #endif
}

void serialWriteSpin() {
    Serial.write(spin);
    #ifdef DEBUG_OUTPUT
      Serial.print("LCurrent spin count: " + (String) spin + "\n");
    #endif
}

void setWifiMode() {
  #ifdef DEBUG_OUTPUT
    Serial.print("LSwitching to WIFI ...\n");
  #endif
  SerialBT.end();
  digitalWrite(LED_BLUE, LOW);
  WiFi.softAP(ssid);
  server.begin();
  prevMillis = millis();
  #ifdef DEBUG_OUTPUT
    Serial.print("LIn WIFI mode - " + (String) ssid + "\n");
  #endif
}

void setBluetoothMode() {
  #ifdef DEBUG_OUTPUT
    Serial.print("LSwitching to Bluetooth ...\n");
  #endif
  server.stop();
  WiFi.mode(WIFI_OFF);
  SerialBT.begin(("ETAG_" + String(chipID)));
  digitalWrite(LED_BLUE, HIGH);
  #ifdef DEBUG_OUTPUT
    Serial.print("LIn Bluetooth mode - ETAG_" + String(chipID) + "\n");
  #endif
}

// Callback for the embedded jquery.min.js page
void onJavaScript(void) {
		server.setContentLength(jquery_min_js_v3_2_1_gz_len);
		server.sendHeader(F("Content-Encoding"), F("gzip"));
    server.send_P(200, "text/javascript", jquery_min_js_v3_2_1_gz, jquery_min_js_v3_2_1_gz_len);
}

void IRAM_ATTR spinCount() {
  spin ++;
  #ifdef DEBUG_OUTPUT
    Serial.println("spin interrupt");
  #endif
}

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);

  for (int i = 0; i < 17; i = i + 8) {
    chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  Serial.begin(115200);
  SerialBT.begin(("ETAG_" + String(chipID)));

  #ifdef DEBUG_OUTPUT
    Serial.print("LBoard starting ...\n");
  #endif

  pinMode(SWSENSE, INPUT);
  pinMode(HALL, INPUT);
  attachInterrupt(HALL, spinCount, RISING);
  
  pinMode(REL_EN, OUTPUT);
  digitalWrite(REL_EN, !releaseFlag);

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, ledState);

  pinMode(SWDRIVE, OUTPUT);
  digitalWrite(SWDRIVE, LOW);

  pinMode(PRES_CS, OUTPUT);
  digitalWrite(PRES_CS, HIGH);

  pressSens.init(PRES_CS);

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
        // Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // flashing firmware to ESP
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        // Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
      } else {
        // Update.printError(Serial);
      }
    }
  });
  #ifdef DEBUG_OUTPUT
    Serial.print("LIn Bluetooth mode - ETAG_" + String(chipID) + "\n");
  #endif
}

void loop() {
  if (WiFi.getMode() != WIFI_OFF) {
    server.handleClient();
    currMillis = millis();
    if (currMillis - prevMillis >= interval) {
      prevMillis = currMillis;
      ledState = !ledState;
      digitalWrite(LED_BLUE, ledState);
    }
  }

  if (Serial.available()) {
    cmd = Serial.read();
    switch (cmd) {
      case ('a'):   // enable release
        enableRelease(true);
        break;
      case ('e'):   // disable release
        enableRelease(false);
        break;
      case ('k'):   // enable saltwater sensor
        enableSaltwaterSensor(true);
        break;
      case ('l'):   // disable saltwater sensor
        enableSaltwaterSensor(false);
        break;
      case ('o'):   // turn off all wireless communication
        turnOffComms();
        break;
      case ('p'):   // read pressure and temperature sensor
        serialWritePressureTemperature();
        break;
      case ('r'):   // reset pressure sensor
        resetPressure();
        break;
      case ('v'):   // read speed sensor
        serialWriteSpin();
        break;
      case ('w'):   // switch to WiFi communication - 192.168.4.1
        setWifiMode();
        break;
      case ('y'):   // switch to Bluetooth communication
        setBluetoothMode();
        break;
      case ('z'):
        serialWriteSaltwaterSensor();
      default:
        break;
    }
  }
  
  if (SerialBT.available()) {
    cmdBT = SerialBT.read();
    switch (cmdBT) {
      case ('a'):   // enable release
        enableRelease(true);
        break;
      case ('e'):   // disable release
        enableRelease(false);
        break;
      case ('k'):   // enable saltwater sensor
        enableSaltwaterSensor(true);
        serialWriteSaltwaterSensor();
        break;
      case ('l'):   // disable saltwater sensor
        digitalWrite(SWDRIVE, LOW);
        break;
      case ('o'):   // turn off all wireless communication
        turnOffComms();
        break;
      case ('p'):   // read pressure and temperature sensor
        serialWritePressureTemperature();
        break;
      case ('v'):   // read speed sensor
        serialWriteSpin();
        break;
      case ('w'):   // switch to WiFi communication - 192.168.4.1
        setWifiMode();
        break;
      case ('z'):
        serialWriteSaltwaterSensor();
      default:
        Serial.write(cmdBT);
        break;
    }
  }

  if (!pressSens.data_ready) {
    pressSens.update_pt_data();
  }
}
