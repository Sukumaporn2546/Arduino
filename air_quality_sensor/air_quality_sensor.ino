#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SensirionI2CSen5x.h>

const char* ssid = "1921131";
const char* password = "25122546";
const char* mqttServer = "demo.thingsboard.io";  // หรือ IP Server
const int mqttPort = 1883;
const char* token = "DfzMlDzDUZCNuoRlVxJ9"; // ของอุปกรณ์ใน ThingsBoard

WiFiClient espClient;
PubSubClient client(espClient);
SensirionI2CSen5x sen5x;

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  // แสดง IP ของบอร์ด
  Serial.println("WiFi connected.");
}

void connectMQTT() {
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32", token, NULL)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  connectWiFi();
  connectMQTT();

  sen5x.begin(Wire);
  sen5x.deviceReset();
  sen5x.startMeasurement();
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  float pm1, pm2_5, pm4, pm10, humidity, temp, voc, nox;
  uint16_t error = sen5x.readMeasuredValues(pm1, pm2_5, pm4, pm10, humidity, temp, voc, nox);
  if (!error) {
    String payload = "{";
    payload += "\"pm1_0\":" + String(pm1, 2) + ",";
    payload += "\"pm2_5\":" + String(pm2_5, 2) + ",";
    payload += "\"pm4_0\":" + String(pm4, 2) + ",";
    payload += "\"pm10\":" + String(pm10, 2) + ",";
    payload += "\"humidity\":" + String(humidity, 2) + ",";
    payload += "\"temperature\":" + String(temp, 2) + ",";
    payload += "\"voc_index\":" + String(voc, 2) + ",";
    payload += "\"nox_index\":" + String(nox, 2);
    payload += "}";

    client.publish("v1/devices/me/telemetry", payload.c_str());
    Serial.println("Sent to ThingsBoard: " + payload);
  } else {
    Serial.println("Error reading sensor data.");
  }

  delay(5000);
}
