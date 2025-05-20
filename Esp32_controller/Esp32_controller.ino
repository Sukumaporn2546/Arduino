#include <WiFi.h>
#include <PubSubClient.h>

// ⚙️ ตั้งค่า WiFi
const char* ssid = "1921131";
const char* password = "25122546";

// ⚙️ Token จาก ThingsBoard ของอุปกรณ์ตัวที่ 2
const char* thingsboardServer = "demo.thingsboard.io";  // หรือ IP/Domain ของ Server ที่คุณใช้
const char* token = "003nT56sCLZAUWJSi0pu";

WiFiClient espClient;
PubSubClient client(espClient);

// ⚙️ GPIO ที่ควบคุม LED
const int LED_PIN = 2; // LED บนบอร์ด ESP32

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // ปิดไฟเริ่มต้น

  // เชื่อมต่อ WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // ตั้งค่า MQTT
  client.setServer(thingsboardServer, 1883);
  client.setCallback(callback);

  reconnect();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (client.connect("ESP32_Client", token, nullptr)) {
      Serial.println("connected");
      // subscribe เพื่อรอรับ RPC
      client.subscribe("v1/devices/me/rpc/request/+");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("RPC Received:");
  Serial.print("Topic: "); 
  Serial.println(topic);
  String rpcPayload;
  for (unsigned int i = 0; i < length; i++) {
    rpcPayload += (char)payload[i];
  }
  Serial.println(rpcPayload);

  // ตรวจสอบว่า method คือ setLed หรือไม่
  if (rpcPayload.indexOf("\"method\":\"setValue\"") != -1) {
    if (rpcPayload.indexOf("\"params\":true") != -1) {
      digitalWrite(LED_PIN, HIGH); // เปิดไฟ
      Serial.println("LED ON");
    } else {
      digitalWrite(LED_PIN, LOW); // ปิดไฟ
      Serial.println("LED OFF");
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
