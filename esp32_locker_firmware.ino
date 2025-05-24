/*
  ESP32 Locker Prototype Firmware
  Features:
  - Magnetic sensor (door open/close)
  - Infrared sensor (presence detection)
  - Buzzer
  - Relay (to control locker lock)
  - 4x4 Keypad for input
  - RTC (Real Time Clock)
  - Red and Green LEDs for status indication
  - Integration with backend via HTTP REST API

  Note: Adjust pin assignments as per your hardware setup.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <RTClib.h>
#include <Keypad.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Backend API endpoint
const char* backendUrl = "http://your-backend-api-url/api/locker"; // Replace with your backend URL

// Pin definitions
const int magneticSensorPin = 34; // GPIO34 input (door sensor)
const int infraredSensorPin = 35; // GPIO35 input (infrared sensor)
const int buzzerPin = 25;         // GPIO25 output
const int relayPin = 26;          // GPIO26 output (locker lock control)
const int redLedPin = 27;         // GPIO27 output
const int greenLedPin = 14;       // GPIO14 output

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {32, 33, 18, 19}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {23, 22, 21, 4};  // Connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// RTC setup
RTC_DS3231 rtc;

// Variables
String inputCode = "";
bool lockerLocked = true;

void setup() {
  Serial.begin(115200);

  pinMode(magneticSensorPin, INPUT);
  pinMode(infraredSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  digitalWrite(relayPin, HIGH); // Assume HIGH means locked
  digitalWrite(redLedPin, HIGH); // Red LED on means locked
  digitalWrite(greenLedPin, LOW);

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Connect to WiFi
  connectWiFi();
}

void loop() {
  // Read sensors
  bool doorClosed = digitalRead(magneticSensorPin) == LOW; // Assuming LOW means door closed
  bool presenceDetected = digitalRead(infraredSensorPin) == HIGH;

  // Read keypad input
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      // Submit code
      if (inputCode.length() > 0) {
        Serial.print("Code entered: ");
        Serial.println(inputCode);
        verifyCodeWithBackend(inputCode);
        inputCode = "";
      }
    } else if (key == '*') {
      // Clear input
      inputCode = "";
      Serial.println("Input cleared");
    } else {
      // Append key to input code
      if (inputCode.length() < 8) { // max code length
        inputCode += key;
        Serial.print("Input code: ");
        Serial.println(inputCode);
      }
    }
  }

  // Update LEDs based on locker state
  if (lockerLocked) {
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
  } else {
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
  }

  delay(100);
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
    delay(500);
    Serial.print(".");
    retryCount++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}

void verifyCodeWithBackend(String code) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    buzzError();
    return;
  }

  HTTPClient http;
  String url = String(backendUrl) + "/verifyCode";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Prepare JSON payload
  String payload = "{\"code\":\"" + code + "\"}";

  int httpResponseCode = http.POST(payload);
  if (httpResponseCode == 200) {
    String response = http.getString();
    Serial.print("Backend response: ");
    Serial.println(response);
    // Parse response and act accordingly
    if (response.indexOf("valid") >= 0) {
      unlockLocker();
    } else {
      buzzError();
    }
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpResponseCode);
    buzzError();
  }
  http.end();
}

void unlockLocker() {
  Serial.println("Unlocking locker");
  digitalWrite(relayPin, LOW); // Activate relay to unlock
  lockerLocked = false;
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, HIGH);
  buzzSuccess();
  delay(5000); // Keep unlocked for 5 seconds
  lockLocker();
}

void lockLocker() {
  Serial.println("Locking locker");
  digitalWrite(relayPin, HIGH); // Deactivate relay to lock
  lockerLocked = true;
  digitalWrite(redLedPin, HIGH);
  digitalWrite(greenLedPin, LOW);
}

void buzzSuccess() {
  tone(buzzerPin, 1000, 200);
  delay(250);
  tone(buzzerPin, 1500, 200);
  delay(250);
}

void buzzError() {
  for (int i = 0; i < 3; i++) {
    tone(buzzerPin, 400, 150);
    delay(200);
  }
}
