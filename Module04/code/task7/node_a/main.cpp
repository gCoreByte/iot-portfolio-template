// Node A – Outside the facility
// Connects to WiFi + MQTT, forwards messages via SoftwareSerial (GPIO25/26) to Node B

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

const char* ssid = "IOT44";
const char* password = "iotempire";
const char* mqttServer = "192.168.14.1";
const int mqttPort = 1883;
const char* mqttTopic = "prison/security";

#define SW_RX 25
#define SW_TX 26

SoftwareSerial swSerial(SW_RX, SW_TX);

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("--- CALLBACK TRIGGERED ---");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Payload length: ");
  Serial.println(length);

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message: [");
  Serial.print(message);
  Serial.println("]");

  // Forward to Node B via SoftwareSerial
  swSerial.println(message);
  Serial.println("Forwarded to Node B");
}

void connectWiFi() {
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);

  Serial.print("Connecting to WiFi (SSID: ");
  Serial.print(ssid);
  Serial.print(")");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts % 10 == 0) {
      Serial.print(" [status=");
      Serial.print(WiFi.status());
      Serial.print("]");
    }
    if (attempts >= 40) {
      Serial.println("\nWiFi failed after 20s, restarting...");
      ESP.restart();
    }
  }
  Serial.println();
  Serial.print("Connected, IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("WiFi RSSI: ");
  Serial.println(WiFi.RSSI());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqtt.connect("NodeA_Outside")) {
      Serial.println("connected!");
      bool subOk = mqtt.subscribe(mqttTopic);
      Serial.print("Subscribe to ");
      Serial.print(mqttTopic);
      Serial.print(": ");
      Serial.println(subOk ? "OK" : "FAILED");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" retrying in 2s");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  swSerial.begin(9600);

  Serial.println("Node A - Outside facility");
  connectWiFi();

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(mqttCallback);
  connectMQTT();
}

unsigned long lastHeartbeat = 0;

void loop() {
  if (!mqtt.connected()) {
    Serial.println("MQTT disconnected, reconnecting...");
    connectMQTT();
  }
  mqtt.loop();

  // Send test message every 5s to verify serial link
  // if (millis() - lastHeartbeat > 5000) {
  //   lastHeartbeat = millis();
  //   Serial.println("[heartbeat] Sending test_ping via SoftwareSerial...");
    //swSerial.println("test_ping");
  // }
}
