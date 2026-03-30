#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= PIN DEFINITIONS =================
#define DHTPIN 4
#define DHTTYPE DHT11

#define RED_LED 26
#define BUZZER 27

// ================= OBJECTS =================
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= CLOUD VARIABLES =================
float temperature;
float humidity;
bool sensorStatus;
bool anomalyDetected;

// ================= WIFI CREDENTIALS =================
const char SSID[] = "smart";
const char PASS[] = "smart@123";

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

// ===================================================
//            MACHINE LEARNING MODEL FUNCTION
// ===================================================
// Extracted from trained Random Forest model

bool predictAnomaly(float t, float h) {

  // Rule 1: Extreme High Temperature OR Very Low Humidity
  if (t > 40 || h < 25) {
    return true;
  }

  // Rule 2: Very Low Temperature OR Very High Humidity
  if (t < 15 || h > 90) {
    return true;
  }

  // Otherwise Normal
  return false;
}

// ===================================================

void setup() {

  Serial.begin(9600);

  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  lcd.init();
  lcd.backlight();

  dht.begin();

  // Initialize Cloud
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  lcd.setCursor(0, 0);
  lcd.print("ML IoT System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {

  ArduinoCloud.update();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // ================= SENSOR FAILURE CHECK =================
  if (isnan(t) || isnan(h)) {

    sensorStatus = false;
    anomalyDetected = true;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT11 FAILURE!");
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring");

    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);

    Serial.println("Sensor Failure Detected!");

  }
  else {

    sensorStatus = true;
    temperature = t;
    humidity = h;

    // ================= ML PREDICTION =================
    bool result = predictAnomaly(t, h);

    if (result) {

      anomalyDetected = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ML ALERT!");
      lcd.setCursor(0, 1);
      lcd.print("Abnormal Data");

      digitalWrite(RED_LED, HIGH);
      digitalWrite(BUZZER, HIGH);

      Serial.println("Anomaly Detected by ML Model");

    }
    else {

      anomalyDetected = false;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(t);
      lcd.print(" C");

      lcd.setCursor(0, 1);
      lcd.print("Hum: ");
      lcd.print(h);
      lcd.print(" %");

      digitalWrite(RED_LED, LOW);
      digitalWrite(BUZZER, LOW);

      Serial.println("Normal Condition");
    }
  }

  delay(2000);
}