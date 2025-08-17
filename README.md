Here’s a clean and professional **README.md** version of your Smart Irrigation System project:

---

# 🌱 Smart Irrigation System using ESP32 and Blynk

This project creates an **automated irrigation system** that intelligently waters plants based on real-time soil moisture levels. It also monitors ambient temperature, humidity, and reservoir water level. The system can be controlled and monitored remotely via the **Blynk IoT platform** on a smartphone.

---

## 🚀 Features

* **Automatic Watering**: Activates the water pump when soil moisture drops below a set threshold.
* **Remote Monitoring**: View live data for temperature, humidity, soil moisture, and water level on the Blynk app.
* **Dual Mode Operation**:

  * *Automatic Mode*: Watering managed based on sensor data.
  * *Manual Mode*: Pump can be controlled via the Blynk app or a physical button.
* **Water Level Safety**: Prevents pump operation when the reservoir is low.
* **LCD Display**: A local 16x2 I2C LCD displays real-time sensor data and system status.
* **Blynk Integration**: Provides a user-friendly mobile and web dashboard.

---

## 🛠️ Hardware Required

* **ESP32 Development Board**
* **Sensors**:

  * DHT11 (Temperature & Humidity)
  * Capacitive Soil Moisture Sensor
  * Water Level Sensor
* **Actuator**: 5V Single Channel Relay Module
* **Display**: 16x2 I2C LCD
* **Input**: Push Button (manual control)
* **Pump**: 5V/6V DC Water Pump
* **Other Components**: Breadboard, jumper wires, vinyl tubing, water reservoir

---

## 💻 Software & Services

* **Arduino IDE** – for programming the ESP32
* **Blynk IoT** – for mobile & web dashboard
* **Libraries Used**:

  * `BlynkSimpleEsp32`
  * `WiFi`
  * `Wire`
  * `LiquidCrystal_I2C`
  * `DHT` (Adafruit)

---

## 🔌 Circuit & Wiring

| Component       | ESP32 Pin   |
| --------------- | ----------- |
| DHT11 Sensor    | GPIO 4 (D4) |
| Soil Moisture   | GPIO 34     |
| Water Level     | GPIO 35     |
| Relay Module IN | GPIO 5 (D5) |
| Manual Button   | GPIO 18     |
| I2C LCD SDA     | GPIO 21     |
| I2C LCD SCL     | GPIO 22     |

**Power:**

* Connect all sensor VCC & LCD to 5V, GND to ESP32 GND.
* Power relay module & pump from a separate 5V source (common ground with ESP32).

---

## 📱 Blynk App Setup

1. **Create Template**:

   * Name: *Smart Irrigation System*
   * Hardware: *ESP32*
   * Copy `BLYNK_TEMPLATE_ID` & `BLYNK_TEMPLATE_NAME` into the code

2. **Configure Datastreams**:

   * V0 → Temperature (°C)
   * V1 → Humidity (%)
   * V2 → Soil Moisture (%)
   * V3 → Water Level (0–4095)
   * V4 → Pump Status
   * V5 → Mode Switch (0 = Manual, 1 = Auto)
   * V6 → Manual Water Control

3. **Dashboard Design**:

   * Gauges: Temperature, Humidity, Soil Moisture
   * Label/Value: Pump Status
   * Switch: Mode Switch (V5)
   * Button: Manual Water (V6, Push mode)

4. **Add Device**:

   * Link to template → Copy generated `BLYNK_AUTH_TOKEN` into the code

---

## ⚙️ Installation

1. Install **ESP32 board manager** in Arduino IDE
2. Install required libraries
3. Update credentials in code:

   ```cpp
   #define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
   #define BLYNK_AUTH_TOKEN  "YOUR_BLYNK_AUTH_TOKEN"
   char ssid[] = "YOUR_WIFI_SSID";
   char pass[] = "YOUR_WIFI_PASSWORD";
   ```
4. Upload the sketch to ESP32

---

## 🌍 How to Use

* **Automatic Mode**: Default mode; waters plants automatically if soil is dry and reservoir has enough water.
* **Manual Mode**:

  * Toggle via Blynk app switch
  * Control pump using:

    * Physical push button
    * Blynk app button (V6)
* Pump runs for a fixed duration (e.g., 5s) per manual activation

---

## 🔧 Sensor Calibration

**Soil Moisture Sensor**

* Dry (air) value ≈ 4095
* Wet (water) value ≈ 1000
* Update mapping:

  ```cpp
  int soilMoisturePercent = map(value, 4095, 1000, 0, 100);
  ```

**Water Level Sensor**

* Empty tank ≈ 3000 (example)
* Set threshold in code:

  ```cpp
  const int waterLevelLowThreshold = 2500;
  ```
