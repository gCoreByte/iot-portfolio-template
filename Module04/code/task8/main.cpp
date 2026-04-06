#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

const char* WIFI_SSID     = "MartinH";
const char* WIFI_PASSWORD = "Karupoeg12";

const char* MQTT_BROKER   = "10.116.68.12";
const int   MQTT_PORT     = 1883;
const char* MQTT_TOPIC    = "iot-mod4/task8/display";
const char* MQTT_CLIENT_ID = "esp32minikit-oled";

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

char lastMessage[128] = "Waiting...";

void displayMessage(const char* msg) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tr);  // 5x7 pixel font

  const int lineHeight = 9;
  const int maxWidth = 64;
  int y = 8;

  const char* p = msg;
  while (*p && y <= 48) {
    // Determine how many characters fit on this line
    int len = strlen(p);
    int fit = len;
    for (int i = 1; i <= len; i++) {
      char buf[128];
      strncpy(buf, p, i);
      buf[i] = '\0';
      if (u8g2.getStrWidth(buf) > maxWidth) {
        fit = i - 1;
        break;
      }
    }
    if (fit == 0) fit = 1;  // at least one char

    // Draw this line
    char line[128];
    strncpy(line, p, fit);
    line[fit] = '\0';
    u8g2.drawStr(0, y, line);

    p += fit;
    y += lineHeight;
  }

  u8g2.sendBuffer();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  unsigned int copyLen = length < sizeof(lastMessage) - 1 ? length : sizeof(lastMessage) - 1;
  memcpy(lastMessage, payload, copyLen);
  lastMessage[copyLen] = '\0';

  Serial.printf("MQTT [%s]: %s\n", topic, lastMessage);
  displayMessage(lastMessage);
}

void connectWiFi() {
  Serial.printf("Connecting to WiFi: %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected — IP: %s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.printf("Connecting to MQTT broker %s ...\n", MQTT_BROKER);
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("MQTT connected");
      mqttClient.subscribe(MQTT_TOPIC);
      Serial.printf("Subscribed to: %s\n", MQTT_TOPIC);
    } else {
      Serial.printf("MQTT failed (rc=%d), retrying in 5s\n", mqttClient.state());
      delay(5000);
    }
  }
}

void setupOTA() {
  ArduinoOTA.setHostname("esp32minikit-oled");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA update starting...");
    displayMessage("OTA Update...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA update complete");
    displayMessage("OTA Done!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if      (error == OTA_AUTH_ERROR)    Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)     Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  u8g2.begin();
  displayMessage("Booting...");

  connectWiFi();
  displayMessage(WiFi.localIP().toString().c_str());
  delay(1500);

  setupOTA();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  connectMQTT();

  displayMessage("Ready!\nTopic:\niot-mod4/\ntask8/display");
}

void loop() {
  ArduinoOTA.handle();

  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();
}
