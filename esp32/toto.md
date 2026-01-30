// Simu données
float temperature = 20.0;
float humidity = 50.0;

void setup() {
  Serial.begin(115200);
  Serial.println("Démarrage de la simulation...");
}

void loop() {
  float tempVariation = random(-10, 46) / 10.0;
  float humVariation = random(-10, 11) / 10.0;

  temperature += tempVariation;
  humidity += humVariation;

  if (temperature > 40.0)
    temperature = 40.0;
  if (temperature < 10.0)
    temperature = 10.0;
  if (humidity > 100.0)
    humidity = 100.0;
  if (humidity < 0.0)
    humidity = 0.0;

  Serial.print("Simulation -> Temp: ");
  Serial.print(temperature);
  Serial.print(" °C | Hum: ");
  Serial.print(humidity);
  Serial.println(" %");

  delay(2000); // 2 secondess
}