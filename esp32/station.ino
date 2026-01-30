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

// Simulation Variables
float tempCelsius = 20.0;
float humidity = 50.0;
bool isCelsius = true;
bool lastButtonState = HIGH;
// Timing for simulation
unsigned long previousMillisData = 0;
const long intervalData = 1000;

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

// JSON Message
String createDataJson(float temperature, float humidity, char unit) {
  StaticJsonDocument<200> doc;

  doc["id"] = "esp32-01";
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["unit"] = String(unit);

  String output;
  serializeJson(doc, output);

  return output;
}

// Publish Weather Data
void publishWeatherData(float temperature, float humidity, char unit) {
  if (!mqttClient.connected()) {
    connectMQTT();
  }

  String tempStr = String(temperature, 1);
  String humStr = String(humidity, 1);
  String unitStr = String(unit);

  // Publish JSON to data topic
  mqttClient.publish(TOPIC_DATA, createDataJson(temperature, humidity, unit).c_str());

  // Debug output
  Serial.println("Published:");
  Serial.println("  Temperature: " + tempStr + " °" + unitStr);
  Serial.println("  Humidity: " + humStr + " %");
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
  if (currentTime - lastPublishTime >= PUBLISH_INTERVAL) {
    // Simulated data
    float temp = 22.0 + random(-30, 30) / 10.0;
    float humidity = 60.0 + random(-100, 100) / 10.0;
    char unit = 'C';

    publishWeatherData(temp, humidity, unit);

    lastPublishTime = currentTime;
  }

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
      tempCelsius += random(-5, 6) / 10.0;
      humidity += random(-10, 11) / 10.0;

      if (tempCelsius > 40.0) tempCelsius = 40.0;
      if (tempCelsius < 10.0) tempCelsius = 10.0;
      if (humidity > 100.0) humidity = 100.0;
      if (humidity < 0.0) humidity = 0.0;

      if (isCelsius) {
        publishWeatherData(tempCelsius, humidity, 'C');
      } else {
        float tempFahrenheit = (tempCelsius * 1.8) + 32.0;
        publishWeatherData(tempFahrenheit, humidity, 'F');
              Serial.print("Temp: ");
      Serial.print(tempF);
      Serial.print(" °F");
      }

      Serial.print(" | Hum: ");
      Serial.print(humidity);
      Serial.println(" %");

      previousMillisData = currentMillis;
    }
}