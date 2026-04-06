#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid       = "MartinH";
const char* password   = "Karupoeg12";
const char* mqttServer = "10.116.68.12";
const int   mqttPort   = 1883;

const int redPin   = 27;
const int greenPin = 25;
const int bluePin  = 32;

// PWM config
const int pwmFreq       = 5000;
const int pwmResolution = 8; // 0-255

// PWM channels
const int redChannel   = 0;
const int greenChannel = 1;
const int blueChannel  = 2;

// Smooth fade state
int targetR = 0, targetG = 0, targetB = 0;
int currentR = 0, currentG = 0, currentB = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setRGB(int r, int g, int b) {
  ledcWrite(redChannel, r);
  ledcWrite(greenChannel, g);
  ledcWrite(blueChannel, b);
}

void smoothFade() {
  bool changed = false;
  if (currentR < targetR) { currentR++; changed = true; }
  else if (currentR > targetR) { currentR--; changed = true; }
  if (currentG < targetG) { currentG++; changed = true; }
  else if (currentG > targetG) { currentG--; changed = true; }
  if (currentB < targetB) { currentB++; changed = true; }
  else if (currentB > targetB) { currentB--; changed = true; }
  if (changed) setRGB(currentR, currentG, currentB);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.printf("MQTT [%s]: %s\n", topic, msg.c_str());

  if (msg == "granted") {
    targetR = 255; targetG = 0; targetB = 0;
    Serial.println("Access GRANTED -> Green");
  } else if (msg == "denied") {
    targetR = 0; targetG = 255; targetB = 0;
    Serial.println("Access DENIED -> Red");
  } else if (msg == "off") {
    targetR = 0; targetG = 0; targetB = 0;
    Serial.println("LED off");
  } else {
    // Try parsing "r,g,b" format
    int r, g, b;
    if (sscanf(msg.c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
      targetR = constrain(r, 0, 255);
      targetG = constrain(g, 0, 255);
      targetB = constrain(b, 0, 255);
      Serial.printf("RGB -> %d, %d, %d\n", targetR, targetG, targetB);
    }
  }
}

void connectWiFi() {
  Serial.printf("Connecting to WiFi %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_RGB_LED")) {
      Serial.println("connected");
      client.subscribe("sensor/led");
    } else {
      Serial.printf("failed (rc=%d), retrying in 2s\n", client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Setup PWM channels
  ledcSetup(redChannel, pwmFreq, pwmResolution);
  ledcSetup(greenChannel, pwmFreq, pwmResolution);
  ledcSetup(blueChannel, pwmFreq, pwmResolution);

  ledcAttachPin(redPin, redChannel);
  ledcAttachPin(greenPin, greenChannel);
  ledcAttachPin(bluePin, blueChannel);

  setRGB(0, 0, 0);

  connectWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
  connectMQTT();
}

void loop() {
  if (!client.connected()) connectMQTT();
  client.loop();

  smoothFade();
  delay(4); // ~250 steps/sec -> full fade in ~1 second
}
