#include <Wire.h>
#include "Adafruit_INA3221.h"

#define SDA_PIN 21
#define SCL_PIN 18
#define QR_RX_PIN 1
#define QR_TX_PIN 3

#define NEXTION_RX 16  // ESP32 RX2 (Connect to Nextion TX)
#define NEXTION_TX 17  // ESP32 TX2 (Connect to Nextion RX)

#define BUTTON_1_GREEN_PASS_SAVE 5
#define BUTTON_2_RED_FAIL_RETAKE 19
#define BUTTON_3_YELLOW_START_NEXT 22

#define IR_SENSOR_PIN 2  // The IR sensor signal pin

Adafruit_INA3221 ina3221 = Adafruit_INA3221();
HardwareSerial nextion(1);  // Use UART1 for Nextion

String qrCode = "";  
bool visualInspectionState = 0;
int test_mode = 0;
volatile int pulseCount = 0;
unsigned long lastTime = 0;
int finalRPM = 0;

void rpmCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!ina3221.begin()) {
    Serial.println("Failed to initialize INA3221 sensor!");
    while (1);
  }
  Serial.println("INA3221 Initialized");

  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN), rpmCounter, FALLING);

  initNextion();
  initQr();
  initButton();

  GoToPage(0);  
  delay(1000);
  GoToPage(1);  
  delay(1000);
}

void loop() {
  if (test_mode == 0) {
    GoToPage(2);
    test_mode++;
  }

  if (test_mode == 1) {
    qrCode = readQr();

    if (qrCode.length() > 0) {
      sendStringToNextion(qrCode, 0);
      Serial.println("001");
      delay(2000);
      Serial.println("002");
      test_mode++;
    }

    if (readYellowButton()) {
      Serial.println("Yellow Button (Start/Next) pressed!");
      test_mode++;
    }
  }

  if (test_mode == 2) {
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
    GoToPage(5);
    if (readYellowButton()) {
      Serial.println("Yellow Button (Start/Next) pressed!");
      test_mode++;
    }
  }

  if (test_mode == 4) {
    GoToPage(6);
    delay(3000);
    GoToPage(11);
    test_mode++;
  }

  if (test_mode == 5) {  // RPM measurement mode
    static int rpm = 0;

    if (millis() - lastTime >= 500) { 
      detachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN));  
      rpm = (pulseCount * 60) / 7;  

      Serial.print("Fan RPM: ");
      Serial.println(rpm);
      sendStringToNextion(String(rpm), 3);
      
      pulseCount = 0;
      lastTime = millis();
      attachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN), rpmCounter, FALLING);
    }

    // Read current values from INA3221
    float current = ina3221.getCurrentAmps(1) * 1000; // Convert A to mA
    Serial.print("Channel 1: Current = ");
    Serial.print(current);
    Serial.println(" mA");
    sendStringToNextion(String(current), 2); // Send current to text box 2 on page 11

    if (readGreenButton()) {
      Serial.println("Green Button Pressed! Stopping RPM measurement.");
      detachInterrupt(digitalPinToInterrupt(IR_SENSOR_PIN));
      finalRPM = rpm;
      

      if (finalRPM > 2500 & current > 0.005 ) {
        Serial.println("RPM is greater than 2500, going to Page 7.");
        GoToPage(7);
        sendStringToNextion(String(finalRPM), 0);
        sendStringToNextion(String(current), 1);
      } else {
        Serial.println("RPM is 2500 or less, going to Page 8.");
        GoToPage(8);
        sendStringToNextion(String(finalRPM), 1);
        sendStringToNextion(String(current), 2);
      }

      test_mode++;
    }
  }
}





















