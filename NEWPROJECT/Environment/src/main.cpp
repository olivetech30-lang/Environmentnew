#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 38
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// 🔑 WiFi Credentials
const char* ssid = "Jazz 4G MIFI_BB76";
const char* password = "54917329";

// ✅ Vercel API Endpoints (FULL HTTPS URLs)
const char* vercelUrlLatest = "https://temperaturesensor.vercel.app/api/latest";
const char* vercelUrlReadings = "https://temperaturesensor.vercel.app/api/readings";

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
  delay(2000); // Give network time to settle
}
void sendToVercel(const char* fullUrl, float temp, float hum) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi disconnected");
    return;
  }

  // Use secure client for HTTPS
  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation (safe for Vercel)

  HTTPClient http;
  if (!http.begin(client, fullUrl)) {
    Serial.println("❌ Failed to initialize HTTP client");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  // Build JSON safely
  String json = "{\"temperature\":" + String(temp, 1) + ",\"humidity\":" + String(hum, 1) + "}";
  Serial.println("📤 Sending: " + json); // Debug: see what’s being sent

  int code = http.POST(json);
  if (code > 0) {
    Serial.println("✅ Sent to: " + String(fullUrl) + " | Status: " + String(code));
  } else {
    Serial.println("❌ HTTP Request Failed (Code: " + String(code) + ")");
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
    Serial.println("🔄 Change detected: Temp=" + String(temp) + "°C, Hum=" + String(hum) + "%");
    sendToVercel(vercelUrlLatest, temp, hum);
    sendToVercel(vercelUrlReadings, temp, hum);

    lastTemp = temp;
    lastHum = hum;
  }

  lastSend = millis();
}
