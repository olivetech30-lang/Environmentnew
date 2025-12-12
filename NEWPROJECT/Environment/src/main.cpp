#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>  // ← Required for HTTPS
#include <DHT.h>

#define DHTPIN 38
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// 🔑 WiFi Credentials
const char* ssid = "Jazz 4G MIFI_BB76";
const char* password = "54917329";

// ✅ Vercel Endpoints — NO TRAILING SPACES!
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
  delay(2000); // Stabilize network
}

void sendToVercel(const char* fullUrl, float temp, float hum) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi not connected");
    return;
  }

  // Use secure client and skip certificate validation
  WiFiClientSecure client;
  client.setInsecure(); // ⚠️ Required for ESP32 to connect to Vercel

  HTTPClient http;
  if (!http.begin(client, fullUrl)) {
    Serial.println("❌ HTTP begin failed");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  String json = "{\"temperature\":" + String(temp, 1) + ",\"humidity\":" + String(hum, 1) + "}";
  Serial.println("📤 Sending JSON: " + json);

  int code = http.POST(json);
  if (code == 200) {
    Serial.println("✅ Success: " + String(fullUrl));
  } else {
    Serial.println("❌ Failed with code: " + String(code));
  }

  http.end();
}

void loop() {
  if (millis() - lastSend < SEND_INTERVAL) {
    return;
  }

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
    Serial.println("🔄 Change detected: Temp=" + String(temp, 1) + "°C, Hum=" + String(hum, 1) + "%");
    sendToVercel(vercelUrlLatest, temp, hum);
    sendToVercel(vercelUrlReadings, temp, hum);

    lastTemp = temp;
    lastHum = hum;
  }

  lastSend = millis();
}
