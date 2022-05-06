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

//#define DEBUG_OUTPUT

#define SWSENSE   12
#define HALL      15
#define REL_EN    2
#define LED_BLUE  0
#define SWDRIVE   4
#define PRES_CS   5


#include <SerialCommunicator.h>
BluetoothSerial SerialBT;
Stream* streams[] = {&Serial, &SerialBT};

SerialCommunicator serialComms(streams, 2);

enum ESP32State {
    COMMAND = 0,
    PASSTHROUGH = 1
};
ESP32State esp32State;

volatile int32_t spin = 0;

uint8_t releaseFlag = LOW;

uint32_t chipID = 0;

String ssid_name;
const char *ssid;

uint8_t ledState = HIGH;

int readCommand;
uint8_t cmd, cmdBT;

const long interval = 1000;
unsigned long currMillis, prevMillis;

MS58xx pressSens;

WebServer server(80);

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

void setBluetoothComms();
void setWiFiComms();
void turnOffComms();
void setRelease(bool boolean);
void setSaltwaterSensor(bool boolean);
void serialWriteSaltwaterSensor();
void serialWritePressureTemperature();
void serialWriteSpeedSensor();
void resetPressureSensor();

void setup() {
    for (int i = 0; i < 17; i = i + 8) {
        chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    Serial.begin(115200);
    SerialBT.begin(("ETAG_" + String(chipID)));

    esp32State = COMMAND;

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

    if (esp32State == COMMAND) {
        readCommand = serialComms.read();

        if (readCommand != -1) {
            cmd = (uint8_t) (readCommand & 0xFF);
            switch (cmd) {
                case ('o'):
                    turnOffComms();
                    break;
                case ('w'):
                    setWiFiComms();  // switch to WiFi communication - 192.168.4.1
                    break;
                case ('y'):
                    setBluetoothComms();
                    break;
                case ('a'):
                    setRelease(true);
                    break;
                case ('e'):
                    setRelease(false);
                    break;
                case('k'):
                    setSaltwaterSensor(true);
                    break;
                case('l'):
                    setSaltwaterSensor(false);
                    break;
                case('z'):
                    serialWriteSaltwaterSensor();
                    break;
                case('p'):
                    serialWritePressureTemperature();
                    break;
            }
        }
    } else if (esp32State == PASSTHROUGH) {
        readCommand = serialComms.read_and_passthrough_until_command();

        if (readCommand != -1) {
            cmd = (uint8_t) (readCommand & 0xFF);
            switch (cmd) {
                case ('o'):
                    turnOffComms();
                    break;
                case ('w'):
                    setWiFiComms();  // switch to WiFi communication - 192.168.4.1
                    break;
                case ('y'):
                    setBluetoothComms();
                    break;      
                default:
                    break;            
            }
        }
    }
}

void setBluetoothComms() {
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
    esp32State = PASSTHROUGH;
}

void setWiFiComms() {
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

void turnOffComms() {
    esp32State == COMMAND;
    SerialBT.end();
    server.stop();
    WiFi.mode(WIFI_OFF);
    digitalWrite(LED_BLUE, LOW);
}

void setRelease(bool boolean) {
// switch starts release when driven LOW. Thus if boolean is true, LOW must be written.
// conversely, if the boolean is false, then HIGH must be written to ensure that release is not set
// Note that HIGH is by definition 0x1. true is 1. Thus !true is 0. false is 0. !false is 1.
    digitalWrite(REL_EN, !boolean);
}

void setSaltwaterSensor(bool boolean) {
// if SWDRIVE is driven HIGH, the saltwater sensor is enabled. Thus the boolean can be passed to turn on the saltwater sensor (true = HIGH).
// if SWDRIVE is driven LOW, the saltwater sensor is disabled. Thus the boolean can be passed to turn off the saltwater sensor (false = LOW).
    digitalWrite(SWDRIVE, boolean);
}

void serialWriteSaltwaterSensor() {
    uint32_t read_val = analogRead(SWSENSE);
    serialComms.write(read_val);
}

void serialWritePressureTemperature() {
    pressSens.calc_press_temp();
    serialComms.write(pressSens.pressure_mbar);
    serialComms.write(pressSens.temperature);
    #ifdef DEBUG_OUTPUT
        Serial.print((String) "LPressure: " + pressSens.pressure_mbar + " mbar\n");
        Serial.print((String) "LTemperature: " + pressSens.temperature + " C\n");
    #endif 
}

void serialWriteSpeedSensor() {
    serialComms.write(spin);
    #ifdef DEBUG_OUTPUT
        Serial.print("LCurrent spin count: " + (String) spin + "\n");
    #endif    
}

void resetPressureSensor() {
    pressSens.init(PRES_CS);
    #ifdef DEBUG_OUTPUT
        Serial.print("LPressure sensor reset\n");
    #endif
}


/*
    if (Serial.available()) {
        cmd = Serial.read();
        switch (cmd) {
            case ('a'):   // enable release
                releaseFlag = HIGH;
                digitalWrite(REL_EN, !releaseFlag);
                break;
            case ('e'):   // disable release
                releaseFlag = LOW;
                digitalWrite(REL_EN, !releaseFlag);
                break;
            case ('k'):   // enable saltwater sensor
                digitalWrite(SWDRIVE, HIGH);
                Serial.print(analogRead(SWSENSE));    Serial.print("\n");
                #ifdef DEBUG_OUTPUT
                    Serial.print((String) "SWSENSE: " + analogRead(SWSENSE) + "\n");
                #endif
                break;
            case ('l'):   // disable saltwater sensor
                digitalWrite(SWDRIVE, LOW);
                break;
            case ('o'):   // turn off all wireless communication
                #ifdef DEBUG_OUTPUT
                    Serial.print("LTurning off Bluetooth and WIFI ...\n");
                #endif
                server.stop();
                WiFi.mode(WIFI_OFF);
                SerialBT.end();
                digitalWrite(LED_BLUE, LOW);
                break;
            case ('p'):   // read pressure and temperature sensor
                pressSens.calc_press_temp();
                Serial.print(pressSens.pressure_mbar);  Serial.print("\n");
                Serial.print(pressSens.temperature);    Serial.print("\n");
                #ifdef DEBUG_OUTPUT
                    Serial.print((String) "LPressure: " + pressSens.pressure_mbar + " mbar\n");
                    Serial.print((String) "LTemperature: " + pressSens.temperature + " C\n");
                #endif
                break;
            case ('r'):   // reset pressure sensor
                pressSens.init(PRES_CS);
                #ifdef DEBUG_OUTPUT
                    Serial.print("LPressure sensor reset\n");
                #endif
                break;
            case ('v'):   // read speed sensor
                Serial.print(spin);   Serial.print("\n");
                #ifdef DEBUG_OUTPUT
                    Serial.print("LCurrent spin count: " + (String) spin + "\n");
                #endif
                break;
            case ('w'):   // switch to WiFi communication - 192.168.4.1
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
                break;
            case ('y'):   // switch to Bluetooth communication
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
                break;
            default:
                break;
        }
    }
  
    if (SerialBT.available()) {
        cmdBT = SerialBT.read();
        Serial.write(cmdBT);
        switch (cmdBT) {
            case ('a'):   // enable release
                releaseFlag = HIGH;
                digitalWrite(REL_EN, !releaseFlag);
                break;
            case ('e'):   // disable release
                releaseFlag = LOW;
                digitalWrite(REL_EN, !releaseFlag);
                break;
            case ('k'):   // enable saltwater sensor
                digitalWrite(SWDRIVE, HIGH);
                Serial.print(analogRead(SWSENSE));    Serial.print("\n");
                #ifdef DEBUG_OUTPUT
                    Serial.print((String) "SWSENSE: " + analogRead(SWSENSE) + "\n");
                #endif
                break;
            case ('l'):   // disable saltwater sensor
                digitalWrite(SWDRIVE, LOW);
                break;
            case ('o'):   // turn off all wireless communication
                #ifdef DEBUG_OUTPUT
                    Serial.print("LTurning off Bluetooth and WIFI ...\n");
                #endif
                server.stop();
                WiFi.mode(WIFI_OFF);
                SerialBT.end();
                digitalWrite(LED_BLUE, LOW);
                break;
            case ('p'):   // read pressure and temperature sensor
                pressSens.calc_press_temp();
                Serial.print(pressSens.pressure_mbar);  Serial.print("\n");
                Serial.print(pressSens.temperature);    Serial.print("\n");
                #ifdef DEBUG_OUTPUT
                    Serial.print((String) "LPressure: " + pressSens.pressure_mbar + " mbar\n");
                    Serial.print((String) "LTemperature: " + pressSens.temperature + " C\n");
                #endif
                break;
            case ('v'):   // read speed sensor
                Serial.print(spin);   Serial.print("\n");
                #ifdef DEBUG_OUTPUT
                    Serial.print("LCurrent spin count: " + (String) spin + "\n");
                #endif
                break;
            case ('w'):   // switch to WiFi communication - 192.168.4.1
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
                break;
            default:
                break;
        }
    }
*/