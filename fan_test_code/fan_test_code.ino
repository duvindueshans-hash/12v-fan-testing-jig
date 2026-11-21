#include <Wire.h>
#include "Adafruit_INA3221.h"

//wifi
#include "WiFiProv.h"
#include "WiFi.h"
#include <EEPROM.h>
#include <HTTPClient.h>

#define SDA_PIN 21
#define SCL_PIN 18
#define QR_RX_PIN 1
#define QR_TX_PIN 3
const int MOSFET_GATE = 23;  // Connect the MOSFET gate to pin 23

#define NEXTION_RX 16  // ESP32 RX2 (Connect to Nextion TX)
#define NEXTION_TX 17  // ESP32 TX2 (Connect to Nextion RX)

#define BUTTON_1_GREEN_PASS_SAVE 5
#define BUTTON_2_RED_FAIL_RETAKE 19
#define BUTTON_3_YELLOW_START_NEXT 22

#define IR_SENSOR_PIN 2  // The IR sensor signal pin


String hashKey = "asdfghjkljhgfdlsmsfdfsmfsdfsdfj";
String deviceMac = "00:1A:2B:3C:4D:10";

bool wifi_connected = 0;

String wifi_ssid;
String wifi_password;

#define wifi_led 27

const char *pop = "abcd1234";                              // Proof of possession - otherwise called a PIN - string provided by the device, entered by the user in the phone app
const char *service_name = "bt_Fan_Tester";  // Name of your device (the Espressif apps expects by default device name starting with "Prov_")
const char *service_key = NULL;                            // Password used for SofAP method (NULL = no password needed)
bool reset_provisioned = true;   


#define EEPROM_SSID_ADDR 0
#define EEPROM_PASS_ADDR 32

void SysProvEvent(arduino_event_t *sys_event);

Adafruit_INA3221 ina3221;
HardwareSerial nextion(1);


float finalRPM;         // Example RPM
float finalCurrent;        // Example current
bool drawCurrentState;    // Example states
bool fanSpeedState = true;
bool overallFanState = true;
bool visualInspectionState = true;

String  qrCode = "";

int test_mode = 0;

volatile int pulseCount = 0;
unsigned long lastTime = 0;

//int finalRPM = 0;
//int finalCurrent = 0;

int PassCounter = 0;

void rpmCounter() {
    pulseCount++;
}

void setup() {
    Serial.begin(9600);
    Serial.println("Setup started");
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.println("I2C initialized");

    if (!ina3221.begin()) 
    {
        Serial.println("Failed to initialize INA3221 sensor!");
        while (1);
    }
    Serial.println("INA3221 Initialized");

    pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN), rpmCounter, FALLING);
    Serial.println("IR Sensor initialized");

    pinMode(MOSFET_GATE, OUTPUT);

    Serial.println("Initializing Nextion...");
    initNextion();
    Serial.println("Initializing QR scanner...");
    initQr();
    Serial.println("Initializing buttons...");
    initButton();

    Serial.println("Going to Page 0");
    GoToPage(0);
    delay(1000);
    Serial.println("Going to Page 1");
    GoToPage(1);
    delay(1000);

    //WiFi
  // Initialize EEPROM
  EEPROM.begin(64);  // Ensure this size is large enough for your SSID and password

  // Read stored SSID and password from EEPROM
  String saved_ssid = readStringFromEEPROM(EEPROM_SSID_ADDR);
  String saved_password = readStringFromEEPROM(EEPROM_PASS_ADDR);

  if (saved_ssid.length() > 0 && saved_password.length() > 0) {
    Serial.println("Attempting to connect using saved credentials.");
    const char *ssid = saved_ssid.c_str();
    const char *password = saved_password.c_str();

    int connectstatus = connectWiFi(ssid, password);

    if (connectstatus == 1) 
    {
      //changePage(26);
      digitalWrite(wifi_led, HIGH);
      return;// Successfully connected, exit setup
    } 
    else 
    {
     // changePage(27);
      Serial.println("Failed to connect with saved credentials, entering provisioning mode.");
      digitalWrite(wifi_led, LOW);
    }
  } 
  else 
  {
    Serial.println("No saved credentials found, entering provisioning mode.");
  }




  // Start provisioning if no valid credentials
  WiFi.onEvent(SysProvEvent);
  Serial.println("Begin Provisioning using BLE");
  uint8_t uuid[16] = { 0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
                       0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02 };
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name, service_key, uuid, reset_provisioned);
// #######################################################################################



}

void loop() {
    Serial.println("Loop started");

    if (test_mode == 0) {
        Serial.println("Switching to test mode 1");
        GoToPage(12);
        test_mode++;
    }

    if (test_mode == 1) {
        Serial.println("Reading QR code");
        qrCode = readQr();

        if (qrCode.length() > 0) {
            Serial.println("QR Code detected: " + qrCode);
            sendStringToNextion(qrCode, 0);
            
            
        }

        if (readYellowButton()) {
            Serial.println("Yellow Button (Start/Next) pressed!");
            test_mode++;
        }
    }

    if (test_mode == 2) {
        Serial.println("Visual Inspection Mode");
        GoToPage(10);
        if (readGreenButton()) {
            Serial.println("Green Button (Pass/Save) pressed!");
            visualInspectionState = true;
            GoToPage(3);
            delay(2000);
            test_mode++;
        }
        if (readRedButton()) {
            Serial.println("Red Button (Fail/Retry) pressed!");
            visualInspectionState = false;
            GoToPage(4);
            delay(2000);
            test_mode++;
        }
    }

    if (test_mode == 3) {
        Serial.println("Waiting for next test");
        GoToPage(13);
        if (readYellowButton()) {
            Serial.println("Yellow Button (Start/Next) pressed!");
            test_mode++;
        }
    }

    if (test_mode == 4) {
        Serial.println("Retrying test");
        digitalWrite(MOSFET_GATE, HIGH);
        GoToPage(6);
        delay(3000);
        GoToPage(11);
        test_mode++;
    }

    if (test_mode == 5) {
        Serial.println("Measuring RPM");

        static int rpm = 0;
        if (millis() - lastTime >= 1000) {  // Increase to 1 second
            detachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN));
            rpm = (pulseCount * 60) / (7 * 1.100); // Adjust the divisor
            Serial.print("Fan RPM: ");
            Serial.println(rpm);
            sendStringToNextion(String(rpm), 3);
            pulseCount = 0;
            lastTime = millis();
            attachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN), rpmCounter, FALLING);
        }

        // Measure current on channel 2
        float current = ina3221.getCurrentAmps(2) * 600;
        Serial.print("Channel 2: Current = ");
        Serial.print(current);
        Serial.print(" mA");
        sendStringToNextion(String(current) + " mA", 2); // with a space before mA

   
        //delay(3000);

        PassCounter++;


        if(PassCounter>20){
        

         finalRPM = rpm;
         finalCurrent = current;

      //  if (readGreenButton()) {
            Serial.println("Green Button Pressed! Stopping RPM measurement.");
            detachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN));
            finalRPM = rpm;

            if (finalRPM > 2500 && current > 50) {
                Serial.println("RPM > 2500, Going to Page 7");
                GoToPage(7);
                sendStringToNextion(String(finalRPM), 0);
                sendStringToNextion(String(finalCurrent),1);
                digitalWrite(MOSFET_GATE, LOW);
                test_mode++;
            } else {
                Serial.println("RPM <= 2500, Going to Page 8");
                GoToPage(8);
                sendStringToNextion(String(finalRPM), 1);
                sendStringToNextion(String(finalCurrent),2);

                digitalWrite(MOSFET_GATE, LOW);
                test_mode++;
            }

            PassCounter = 0;
        }
    }

    if (test_mode == 6) {  // Moved outside of test_mode 5 block
        if (readRedButton()) {
            Serial.println("Red button pressed, retrying test.");
            GoToPage(12);
            test_mode = 1;
        }

        if (readGreenButton()) {
            Serial.println("Green button pressed, proceeding to next step.");
            GoToPage(14);
            delay(1000);
            GoToPage(9);
            delay(1000);
            GoToPage(15);
            delay(1000);
            test_mode = 0;
            
        }
    }

            Serial.print(" PassCounter-");
        Serial.println(PassCounter);
}



