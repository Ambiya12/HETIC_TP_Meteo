#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "DHT.h";

// WiFi
const char *WIFI_SSID = "Galyst";
const char *WIFI_PASSWORD = "galystann";

// MQTT
const char *MQTT_SERVER = "captain.dev0.pandor.cloud";
const int MQTT_PORT = 1884;
const char *MQTT_CLIENT_ID = "ESP32-Weather-Station";

// MQTT Topics
const char *TOPIC_DATA = "station-meteo/data";
const char *TOPIC_SET_UNIT = "station-meteo/set-unit";

// Timing for periodic publishing
unsigned long lastPublishTime = 0;
const unsigned long PUBLISH_INTERVAL = 1000;

// DHT Sensor
#define DHTPIN 17
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Pins
const int LED_PIN_C = 13;
const int LED_PIN_F = 14;
const int BUTTON_PIN = 16;

// State
bool isCelsius = true;
bool lastButtonState = HIGH;
unsigned long previousMillisData = 0;
const long intervalData = 1000;

// Simulation (fallback si capteur absent)
float simTemperature = 22.0;
float simHumidity = 55.0;
bool useSensorData = true;

// WiFi and MQTT Clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// WiFi Connection
void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT Setup
void setupMQTT() { mqttClient.setServer(MQTT_SERVER, MQTT_PORT); }

// MQTT Connection with Reconnection Logic
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");

    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Simulation de données météo (fallback si capteur absent)
void simulateWeatherData(float &temperature, float &humidity, char unit) {
  simTemperature += random(-5, 6) / 10.0;
  if (simTemperature > 40.0) simTemperature = 40.0;
  if (simTemperature < -10.0) simTemperature = -10.0;

  simHumidity += random(-10, 11) / 10.0;
  if (simHumidity > 100.0) simHumidity = 100.0;
  if (simHumidity < 0.0) simHumidity = 0.0;

  if (unit == 'F') {
    temperature = simTemperature * 1.8 + 32.0;
  } else {
    temperature = simTemperature;
  }
  humidity = simHumidity;
}

// JSON Message
String createDataJson(float temperature, float humidity, char unit, bool isRealData) {
  StaticJsonDocument<200> doc;

  doc["id"] = "esp32-01";
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["unit"] = String(unit);
  doc["mode"] = isRealData ? "reel" : "simulation";

  String output;
  serializeJson(doc, output);

  return output;
}

// Publish Weather Data
void publishWeatherData(float temperature, float humidity, char unit, bool isRealData) {
  if (!mqttClient.connected()) {
    connectMQTT();
  }

  // Publish JSON to data topic
  mqttClient.publish(TOPIC_DATA, createDataJson(temperature, humidity, unit, isRealData).c_str());

  // Debug output
  Serial.print("[");
  Serial.print(isRealData ? "REEL" : "SIMU");
  Serial.print("] Temp: ");
  Serial.print(temperature, 1);
  Serial.print(" °");
  Serial.print(unit);
  Serial.print(" | Hum: ");
  Serial.print(humidity, 1);
  Serial.println(" %");
}

// Setup
void setup() {
    // Initialize Serial
  Serial.begin(115200);
  // Initialize pins
  pinMode(LED_PIN_C, OUTPUT);
  pinMode(LED_PIN_F, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Initialize DHT sensor
  dht.begin();
  delay(200);
  // Initialize button state
  lastButtonState = digitalRead(BUTTON_PIN);

  Serial.println("\n=== ESP32 Weather Station Starting ===");

  // Connect to WiFi
  setupWiFi();
  // Setup and connect to MQTT
  setupMQTT();
  connectMQTT();

  Serial.println("=== Setup Complete ===\n");
}

// Main Loop
void loop() {
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  unsigned long currentTime = millis();

  unsigned long currentMillis = millis();
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState != lastButtonState && currentButtonState == LOW){
    isCelsius = !isCelsius;
    
    if (isCelsius) {
      digitalWrite(LED_PIN_C, HIGH);
      digitalWrite(LED_PIN_F, LOW);
    } else {
      digitalWrite(LED_PIN_C, LOW);
      digitalWrite(LED_PIN_F, HIGH);
    }
    
    delay(50); 
  }
  lastButtonState = currentButtonState;

  if (currentMillis - previousMillisData >= intervalData) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    char unit = isCelsius ? 'C' : 'F';

    // Si le capteur ne répond pas, utilise la simulation
    if (isnan(temperature) || isnan(humidity)) {
      if (useSensorData) {
        Serial.println("[WARN] Capteur DHT non detecte, mode simulation active");
        useSensorData = false;
      }
      simulateWeatherData(temperature, humidity, unit);
    } else {
      if (!useSensorData) {
        Serial.println("[INFO] Capteur DHT detecte, mode reel active");
        useSensorData = true;
      }
      // Conversion en Fahrenheit si nécessaire
      if (!isCelsius) {
        temperature = temperature * 1.8 + 32;
      }
    }

    // Publication
    publishWeatherData(temperature, humidity, unit, useSensorData);

    previousMillisData = currentMillis;
  }
}