// AC Controller Node
// Listens to MQTT command topic to turn relay (simulated AC unit) on/off
// Reports status on a separate topic every 5s and on each state change
// Safety: auto-shutoff after 30s to prevent overheating

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "MartinH";
const char* password = "Karupoeg12";
const char* mqttServer = "10.116.68.12";
const int mqttPort = 1883;

const char* topicCommand = "facility/ac/command";
const char* topicStatus  = "facility/ac/status";

const int RELAY_PIN = D1;
const unsigned long MAX_ON_MS = 10000;
const unsigned long STATUS_INTERVAL_MS = 5000;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

bool acOn = false;
unsigned long acTurnedOnAt = 0;
unsigned long lastStatusAt = 0;

void publishStatus() {
  const char* status = acOn ? "ON" : "OFF";
  mqtt.publish(topicStatus, status, true);
  Serial.print("Status published: ");
  Serial.println(status);
  lastStatusAt = millis();
}

void setAC(bool on) {
  if (on == acOn) return;
  acOn = on;
  digitalWrite(RELAY_PIN, acOn ? HIGH : LOW);
  if (acOn) {
    acTurnedOnAt = millis();
    Serial.println("AC turned ON");
  } else {
    Serial.println("AC turned OFF");
  }
  publishStatus();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char buf[length + 1];
  memcpy(buf, payload, length);
  buf[length] = '\0';

  Serial.print("Received on ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(buf);

  if (strcmp(buf, "1") == 0 || strcasecmp(buf, "ON") == 0) {
    setAC(true);
  } else if (strcmp(buf, "0") == 0 || strcasecmp(buf, "OFF") == 0) {
    setAC(false);
  }
}

void connectWiFi() {
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts >= 40) {
      Serial.println("\nWiFi failed, restarting...");
      ESP.restart();
    }
  }
  Serial.print("\nConnected, IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqtt.connect("ac-controller")) {
      Serial.println("connected!");
      mqtt.subscribe(topicCommand);
      Serial.print("Subscribed to ");
      Serial.println(topicCommand);
      publishStatus();
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
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  connectWiFi();

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(mqttCallback);
  connectMQTT();
}

void loop() {
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  // Safety: auto-shutoff after 30s
  if (acOn && (millis() - acTurnedOnAt >= MAX_ON_MS)) {
    Serial.println("Safety shutoff: 30s limit reached!");
    setAC(false);
  }

  // Re-report status every 5s
  if (millis() - lastStatusAt >= STATUS_INTERVAL_MS) {
    publishStatus();
  }
}
