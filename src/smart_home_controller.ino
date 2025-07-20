#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ------------------ Pins ------------------
const int lightRelay = 8;
const int fanRelay = 9;
const int gasSensorPin = A0;
const int ldrPin = A1;
const int buzzerPin = 13;
const int servoPin = 7;

// ------------------ Objects ------------------
SoftwareSerial gsm(10, 11); // RX, TX
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

// ------------------ Settings ------------------
const String replyNumber = "+917099641701";  // Your number here
const int gasThreshold = 500;

// ------------------ Variables ------------------
String currentLine = "";
bool expectingSMS = false;
bool gasLeakActive = false;
bool manualFanOverride = false;
bool gasAlertSent = false;

void setup() {
  pinMode(lightRelay, OUTPUT);
  pinMode(fanRelay, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(lightRelay, HIGH); // OFF
  digitalWrite(fanRelay, HIGH);   // OFF

  myServo.attach(servoPin);
  myServo.write(0);

  Serial.begin(9600);
  gsm.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("GSM Smart Home");
  lcd.setCursor(0, 1);
  lcd.print("Waiting for SMS");

  delay(1000);
  gsm.println("AT");
  delay(500);
  gsm.println("AT+CMGF=1"); // Text mode
  delay(500);
  gsm.println("AT+CSCS=\"GSM\"");
  delay(500);
  gsm.println("AT+CNMI=1,2,0,0,0"); // Direct SMS
  delay(500);

  Serial.println("📶 GSM Ready. Waiting for SMS...");
}

String sanitizeAndLog(String input) {
  String clean = "";
  for (int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c >= 'a' && c <= 'z') c -= 32; // toUpper
    if ((c >= 'A' && c <= 'Z')) {
      clean += c;
    }
  }
  return clean;
}

void sendSMS(String message) {
  gsm.println("AT+CMGS=\"" + replyNumber + "\"");
  delay(500);
  gsm.print(message);
  gsm.write(26); // Ctrl+Z
  delay(1000);
  Serial.println("📤 Sent: " + message);
}

void updateLCD(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

String getDeviceStatus() {
  String lightStatus = digitalRead(lightRelay) == LOW ? "Light ON" : "Light OFF";
  String fanStatus = digitalRead(fanRelay) == LOW ? "Fan ON" : "Fan OFF";
  return fanStatus + ", " + lightStatus;
}

void processCommand(String cmd) {
  String cleaned = sanitizeAndLog(cmd);

  if (cleaned == "ONL") {
    digitalWrite(lightRelay, LOW);
    sendSMS("Light turned ON.");
    updateLCD("Light Status:", "ON");
  } else if (cleaned == "OFFL") {
    digitalWrite(lightRelay, HIGH);
    sendSMS("Light turned OFF.");
    updateLCD("Light Status:", "OFF");
  } else if (cleaned == "ONF") {
    if (!gasLeakActive) {
      digitalWrite(fanRelay, LOW);
      manualFanOverride = true;
      sendSMS("Fan turned ON.");
      updateLCD("Fan Status:", "ON");
    } else {
      sendSMS("Cannot turn OFF fan during gas leak!");
    }
  } else if (cleaned == "OFFF") {
    if (!gasLeakActive) {
      digitalWrite(fanRelay, HIGH);
      manualFanOverride = false;
      sendSMS("Fan turned OFF.");
      updateLCD("Fan Status:", "OFF");
    } else {
      sendSMS("Fan cannot be turned OFF during gas leak!");
    }
  } else if (cleaned == "STATUS") {
    String status = getDeviceStatus();
    sendSMS(status);
    updateLCD("Device Status", status.substring(0, 16));
  } else {
    sendSMS("Unknown command.");
    updateLCD("Invalid Command", "");
  }
}

void loop() {
  // Read sensors
  int gasValue = analogRead(gasSensorPin);
  int ldrValue = analogRead(ldrPin);

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("Gas:");
  lcd.print(gasValue);
  lcd.print(" L:");
  lcd.print(ldrValue);

  // GAS LEAK DETECTION
  if (gasValue > gasThreshold) {
    if (!gasLeakActive) {
      gasLeakActive = true;
      tone(buzzerPin, 1000);
      myServo.write(90);
      digitalWrite(fanRelay, LOW); // Turn ON
      manualFanOverride = false;
      gasAlertSent = false;
    }

    if (!gasAlertSent) {
      sendSMS("⚠ Gas Leak Detected! Fan turned ON.");
      gasAlertSent = true;
    }

    updateLCD("🚨 Gas Alert!", "Fan Auto ON");

  } else {
    if (gasLeakActive) {
      gasLeakActive = false;
      noTone(buzzerPin);
      myServo.write(0);
      gasAlertSent = false;

      if (!manualFanOverride) {
        digitalWrite(fanRelay, HIGH); // Turn OFF if auto mode
        updateLCD("✅ Gas Normal", "Fan Auto OFF");
      } else {
        updateLCD("✅ Gas Normal", "Fan Manual ON");
      }
    }
  }

  // SMS Parsing
  while (gsm.available()) {
    char c = gsm.read();
    if ((c >= 32 && c <= 126) || c == '\n' || c == '\r') {
      currentLine += c;
    }

    if (c == '\n' || c == '\r') {
      currentLine.trim();

      if (currentLine.startsWith("+CMT:")) {
        expectingSMS = true;
      } else if (expectingSMS && currentLine.length() > 0) {
        processCommand(currentLine);
        expectingSMS = false;
      }

      currentLine = "";
    }
  }

  delay(500);
}