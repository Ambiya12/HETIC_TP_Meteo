#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// WiFi
const char *WIFI_SSID = "toto";
const char *WIFI_PASSWORD = "toto";

// MQTT
const char *MQTT_SERVER = "captain.dev0.pandor.cloud";
const int MQTT_PORT = 1884;
const char *MQTT_CLIENT_ID = "ESP32-Weather-Station";

// MQTT Topics
const char *TOPIC_TEMPERATURE = "station-meteo/temperature";
const char *TOPIC_HUMIDITY = "station-meteo/humidity";
const char *TOPIC_UNIT = "station-meteo/unit";
const char *TOPIC_SET_UNIT = "station-meteo/set-unit";

// Timing for periodic publishing
unsigned long lastPublishTime = 0;
const unsigned long PUBLISH_INTERVAL = 1000;

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

  doc["id"] = "12";
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

  // Publish to individual topics
    mqttClient.publish(TOPIC_TEMPERATURE, createDataJson(temperature, humidity, unit););
  // String tempStr = String(temperature, 1);
  // String humStr = String(humidity, 1);
  // String unitStr = String(unit);


  // mqttClient.publish(TOPIC_TEMPERATURE, tempStr.c_str());
  // mqttClient.publish(TOPIC_HUMIDITY, humStr.c_str());
  // mqttClient.publish(TOPIC_UNIT, unitStr.c_str());

  // Debug output
  Serial.println("Published:");
  Serial.println("  Temperature: " + tempStr + " °" + unitStr);
  Serial.println("  Humidity: " + humStr + " %");
}

// Setup
void setup() {
  Serial.begin(115200);
  delay(1000);

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
}
