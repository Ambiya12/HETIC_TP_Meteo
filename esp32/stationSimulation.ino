#include "DHT.h";

#define DHTPIN 17
#define DHTTYPE DHT22

const int LED_PIN_C = 13;
const int LED_PIN_F = 14;
const int BUTTON_PIN = 16;

DHT dht(DHTPIN, DHTTYPE);

float tempCelsius = 20.0;
float humidity = 50.0;
bool isCelsius = true;
bool lastButtonState = HIGH;

unsigned long previousMillisData = 0;
const long intervalData = 1000;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN_C, OUTPUT);
  pinMode(LED_PIN_F, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  dht.begin();

  delay(50);
  lastButtonState = digitalRead(BUTTON_PIN);
}

void loop() {
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
      digitalWrite(LED_PIN_C, HIGH);
      digitalWrite(LED_PIN_F, LOW);
      Serial.print("Temp: ");
      Serial.print(tempCelsius);
      Serial.print(" °C");
    } else {
      digitalWrite(LED_PIN_C, LOW);
      digitalWrite(LED_PIN_F, HIGH);
      float tempF = tempCelsius * 1.8 + 32.0;
      Serial.print("Temp: ");
      Serial.print(tempF);
      Serial.print(" °F");
    }

    Serial.print(" | Hum: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}