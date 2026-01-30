# 🌤️ Station Météo ESP32

A real-time weather station system using ESP32, MQTT, WebSocket, and a web frontend.

## 👥 Team Members

- Jules Bourin
- Ambiya Dimas Galystan
- Anatole Dupuis
- Anatole TU
- Myriam Gzara

## 📋 Prerequisites

- Node.js (v14 or higher)
- ESP32 board with DHT22 sensor
- MQTT broker access

## 🚀 How to Run

### 1. Bridge Server (MQTT to WebSocket)

```bash
cd bridge
npm install
npx ts-node server.ts
```

The bridge server will start on port 8080.

### 2. Frontend

```bash
cd frontend
# Or use a local server:
python3 -m http.server 3000

or

npx serve -p 3000
```

Then open `http://localhost:3000` in your browser.

### 3. ESP32

1. Open `esp32/station.ino` in Arduino IDE
2. **Update WiFi credentials** (lines 7-8):
   ```cpp
   const char *WIFI_SSID = "YourWiFiName";
   const char *WIFI_PASSWORD = "YourWiFiPassword";
   ```
   ⚠️ The current values (`Galyst` / `galystann`) are personal credentials. Replace them with your own WiFi network name and password.
3. Upload to your ESP32 board
4. Connect DHT22 sensor to pin 17

## 🔌 Ports

- Bridge WebSocket: `8080`
- MQTT Broker: `captain.dev0.pandor.cloud:1884`
- Frontend: `3000` (if using local server)

## 📊 Features

- Real-time temperature and humidity monitoring
- Celsius/Fahrenheit unit switching
- Data history tracking
- WebSocket live updates
