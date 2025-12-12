#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 38
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// 🔑 Replace with your WiFi and Vercel URL
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
// Use your actual Vercel deployment URL
const char* vercelUrlLatest = "https://climatecloud.vercel.app/api/latest";
const char* vercelUrlReadings = "https://climatecloud.vercel.app/api/readings";

float lastTemp = -999;
float lastHum = -999;
unsigned long lastSend = 0;
const long SEND_INTERVAL = 5000; // 5 seconds

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void sendToVercel(const char* url, float temp, float hum) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"temperature\":" + String(temp, 1) + ",\"humidity\":" + String(hum, 1) + "}";
  int code = http.POST(json);

  if (code == 200) {
    Serial.println("✅ Sent to: " + String(url));
  } else {
    Serial.println("❌ HTTP Error (" + String(url) + "): " + String(code));
  }
  http.end();
}

void loop() {
  if (millis() - lastSend < SEND_INTERVAL) return;

  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(hum) || isnan(temp)) {
    Serial.println("DHT read failed!");
    lastSend = millis();
    return;
  }

  bool tempChanged = abs(temp - lastTemp) > 0.2;
  bool humChanged = abs(hum - lastHum) > 1.0;

  if (tempChanged || humChanged) {
    // Send to both endpoints
    sendToVercel(vercelUrlLatest, temp, hum);
    sendToVercel(vercelUrlReadings, temp, hum);

    lastTemp = temp;
    lastHum = hum;
  }

  lastSend = millis();
}