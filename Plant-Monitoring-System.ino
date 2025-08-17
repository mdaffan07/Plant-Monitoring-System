// --- Blynk Template Information ---
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Irrigation System"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

// --- Include necessary libraries ---
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// --- Pin Definitions ---
#define DHT_PIN 4
#define SOIL_MOISTURE_PIN 34
#define WATER_LEVEL_PIN 35
#define RELAY_PIN 5
#define MANUAL_WATER_BUTTON_PIN 18

// --- Sensor & LCD Configuration ---
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Blynk & WiFi Configuration ---
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// Define Blynk Virtual Pins
#define V_TEMP V0
#define V_HUMIDITY V1
#define V_SOIL_MOISTURE V2
#define V_WATER_LEVEL V3
#define V_PUMP_STATUS V4
#define V_MODE_SWITCH V5
#define V_MANUAL_WATER V6

// --- Global Variables ---
bool isAutoMode = true;
int soilMoistureThreshold = 45;

// --- FIXED: CALIBRATE THIS VALUE! ---
// Higher ADC value typically means less water. Check your sensor's output when water is low.
// A value around 2500-3000 is common for a "low water" state when the sensor is mostly dry.
const int waterLevelLowThreshold = 2500;

// --- Timer variables for non-blocking delays ---
unsigned long lastSensorReadTime = 0;
const long sensorReadInterval = 2000;  // Read sensors every 2 seconds

bool isManualWatering = false;
unsigned long manualWaterStartTime = 0;
const long manualWaterDuration = 5000;  // Water for 5 seconds manually

// This function is called every time the virtual pin for mode switch is changed
BLYNK_WRITE(V_MODE_SWITCH) {
  isAutoMode = param.asInt();
  if (isAutoMode) {
    Serial.println("Mode switched to Automatic");
    lcd.setCursor(0, 1);
    lcd.print("Mode: Automatic ");
  } else {
    Serial.println("Mode switched to Manual");
    isManualWatering = false;      // Stop any manual watering if mode is switched
    digitalWrite(RELAY_PIN, LOW);  // Turn off pump when switching to manual
    Blynk.virtualWrite(V_PUMP_STATUS, "OFF");
    lcd.setCursor(0, 1);
    lcd.print("Mode: Manual    ");
  }
}

// This function is called every time the virtual button for manual watering is pressed
BLYNK_WRITE(V_MANUAL_WATER) {
  if (!isAutoMode && param.asInt() == 1) {
    // Start manual watering if not already in progress
    if (!isManualWatering) {
      manualWater();
    }
  }
}

void setup() {
  Serial.begin(115200);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Garden");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  dht.begin();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MANUAL_WATER_BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PIN, LOW);

  Blynk.begin(auth, ssid, pass);
  lcd.clear();
}

void loop() {
  Blynk.run();  // This must run as often as possible

  unsigned long currentTime = millis();

  // --- Handle Manual Watering Timer ---
  if (isManualWatering && (currentTime - manualWaterStartTime >= manualWaterDuration)) {
    isManualWatering = false;
    digitalWrite(RELAY_PIN, LOW);
    Blynk.virtualWrite(V_PUMP_STATUS, "OFF");
    Serial.println("Manual watering finished.");
    lcd.setCursor(0, 1);
    lcd.print("Mode: Manual    ");
  }

  // --- Handle physical manual water button press ---
  if (digitalRead(MANUAL_WATER_BUTTON_PIN) == LOW && !isAutoMode && !isManualWatering) {
    manualWater();
    delay(200);  // Small debounce delay is acceptable here
  }

  // --- Main sensor read and logic block, runs every 'sensorReadInterval' milliseconds ---
  if (currentTime - lastSensorReadTime >= sensorReadInterval) {
    lastSensorReadTime = currentTime;

    // --- 1. Read all sensor data ---
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
    int waterLevelValue = analogRead(WATER_LEVEL_PIN);

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // --- 2. Process and Map Sensor Data ---
    // Calibrate these values by testing your sensor when dry (e.g., 4095) and wet (e.g., 1000)
    int soilMoisturePercent = map(soilMoistureValue, 4095, 1000, 0, 100);
    soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);

    // --- 3. Print to Serial Monitor ---
    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print("C, Humid: ");
    Serial.print(h);
    Serial.print("%, Soil: ");
    Serial.print(soilMoisturePercent);
    Serial.print("%, Water ADC: ");
    Serial.println(waterLevelValue);

    // --- 4. Send Data to Blynk ---
    Blynk.virtualWrite(V_TEMP, t);
    Blynk.virtualWrite(V_HUMIDITY, h);
    Blynk.virtualWrite(V_SOIL_MOISTURE, soilMoisturePercent);
    Blynk.virtualWrite(V_WATER_LEVEL, waterLevelValue);

    // --- 5. Update LCD Display ---
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(t, 1);
    lcd.print("C  M:");
    lcd.print(soilMoisturePercent);
    lcd.print("%   ");

    // --- 6. Automatic Watering Logic ---
    if (isAutoMode) {
      // FIXED: Check if water level is HIGH (bad)
      if (waterLevelValue >= waterLevelLowThreshold) {
        Serial.println("Water level is low! Pump will not run.");
        digitalWrite(RELAY_PIN, LOW);
        Blynk.virtualWrite(V_PUMP_STATUS, "WATER LOW");
        lcd.setCursor(0, 1);
        lcd.print("Water Level Low!");
      } else if (soilMoisturePercent < soilMoistureThreshold) {
        Serial.println("Soil is dry. Turning pump ON.");
        digitalWrite(RELAY_PIN, HIGH);
        Blynk.virtualWrite(V_PUMP_STATUS, "ON");
        lcd.setCursor(0, 1);
        lcd.print("Watering...     ");
      } else {
        Serial.println("Conditions met. Turning pump OFF.");
        digitalWrite(RELAY_PIN, LOW);
        Blynk.virtualWrite(V_PUMP_STATUS, "OFF");
        lcd.setCursor(0, 1);
        lcd.print("Mode: Automatic ");
      }
    }
  }
}

// Function to handle manual watering
void manualWater() {
  int waterLevelValue = analogRead(WATER_LEVEL_PIN);
  // FIXED: Check if water level is HIGH (bad)
  if (waterLevelValue >= waterLevelLowThreshold) {
    Serial.println("Water level is low! Cannot manually water.");
    Blynk.virtualWrite(V_PUMP_STATUS, "WATER LOW");
    lcd.setCursor(0, 1);
    lcd.print("Water Level Low!");
    delay(2000);  // Using delay here is acceptable as it's an error message
    lcd.setCursor(0, 1);
    lcd.print("Mode: Manual    ");
  } else {
    Serial.println("Manual watering activated.");
    isManualWatering = true;
    manualWaterStartTime = millis();
    digitalWrite(RELAY_PIN, HIGH);
    Blynk.virtualWrite(V_PUMP_STATUS, "ON");
    lcd.setCursor(0, 1);
    lcd.print("Watering...     ");
  }
}