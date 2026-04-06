#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 48
#define OLED_RESET -1
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid       = "MartinH";
const char* password   = "Karupoeg12";
const char* mqttServer = "10.116.68.12";
const int   mqttPort   = 1883;

const char* topicDisplay = "sensor/display";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void showOnDisplay(const char* msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';

  Serial.print("MQTT [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  showOnDisplay(msg);
}

void connectWiFi() {
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);

  Serial.print("Connecting to WiFi");
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
  Serial.print("\nWiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqtt.connect("OLEDDisplayNode")) {
      Serial.println("connected");
      mqtt.subscribe(topicDisplay);
      Serial.print("Subscribed to: ");
      Serial.println(topicDisplay);
      showOnDisplay("Waiting for message...");
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
  delay(100);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 allocation failed!");
    while (true) delay(1000);
  }

  showOnDisplay("Booting...");

  connectWiFi();

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(mqttCallback);
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();
}
