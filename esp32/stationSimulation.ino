// Simulation de données météo pour tests sans capteur DHT22

float simTemperature = 22.0;
float simHumidity = 55.0;

// Génère des données simulées réalistes
void simulateWeatherData(float &temperature, float &humidity, char unit) {
  // Variation aléatoire de la température (-0.5 à +0.5)
  simTemperature += random(-5, 6) / 10.0;

  // Limites réalistes en Celsius
  if (simTemperature > 40.0) simTemperature = 40.0;
  if (simTemperature < -10.0) simTemperature = -10.0;

  // Variation aléatoire de l'humidité (-1 à +1)
  simHumidity += random(-10, 11) / 10.0;

  // Limites humidité (0-100%)
  if (simHumidity > 100.0) simHumidity = 100.0;
  if (simHumidity < 0.0) simHumidity = 0.0;

  // Retourne la température dans l'unité demandée
  if (unit == 'F') {
    temperature = simTemperature * 1.8 + 32.0;
  } else {
    temperature = simTemperature;
  }

  humidity = simHumidity;
}
