#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// WiFi Credentials - Update these!
const char *ssid = "Simmmm:)";
const char *password = "sim10111001";

AsyncWebServer server(80);

// Global state to store incoming sensor data
struct {
  float temp = 0.0;
  int lux = 0;
  int r = 0, g = 0, b = 0;
  int cid = 0;
  uint32_t hi = 0;
  String st = "Idle";
} sensorData;

// HTML Content
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>NEON DASHBOARD</title>
  <style>
    body { font-family: sans-serif; background: #121212; color: #eee; text-align: center; }
    .container { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; padding: 20px; }
    .card { background: #1e1e1e; padding: 20px; border-radius: 10px; border: 1px solid #333; }
    .value { font-size: 2.5rem; color: #03dac6; }
    .label { color: #888; text-transform: uppercase; font-size: 0.8rem; }
    #colorBox { width: 50px; height: 50px; border-radius: 50%; margin: 10px auto; border: 2px solid #444; }
  </style>
</head>
<body>
  <h1>NEON DASHBOARD</h1>
  <div class="container">
    <div class="card"><div class="label">Temperature</div><div class="value" id="temp">--</div></div>
    <div class="card"><div class="label">Luminosity</div><div class="value" id="lux">--</div></div>
    <div class="card"><div class="label">Color</div><div id="colorBox"></div><div id="rgbText">---</div></div>
    <div class="card"><div class="label">Game Status</div><div class="value" id="status">--</div></div>
  </div>

  <script>
    function update() {
      fetch('/data').then(r => r.json()).then(d => {
        document.getElementById('temp').innerText = d.temp.toFixed(1) + "C";
        document.getElementById('lux').innerText = d.lux;
        document.getElementById('status').innerText = d.st;
        document.getElementById('colorBox').style.backgroundColor = `rgb(${d.r},${d.g},${d.b})`;
        document.getElementById('rgbText').innerText = `RGB: (${d.r},${d.g},${d.b})`;
      });
    }
    setInterval(update, 500);
  </script>
</body>
</html>)rawliteral";

void setup() {
  Serial.begin(115200);
  
  // Use GPIO 16 (RX) and 17 (TX) for STM32 communication
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    doc["temp"] = sensorData.temp;
    doc["lux"] = sensorData.lux;
    doc["r"] = sensorData.r;
    doc["g"] = sensorData.g;
    doc["b"] = sensorData.b;
    doc["st"] = sensorData.st;
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  server.begin();
}

void loop() {
  if (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, line);
    if (!err) {
      sensorData.temp = doc["temp"] | 0.0;
      sensorData.lux = doc["lux"] | 0;
      sensorData.r = doc["r"] | 0;
      sensorData.g = doc["g"] | 0;
      sensorData.b = doc["b"] | 0;
      sensorData.st = doc["st"] | "N/A";
    }
  }
}
