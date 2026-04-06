// Node B – Inside the protected facility
// No WiFi. Listens on SoftwareSerial (GPIO25/26) for messages from Node A
// and prints them to the Serial Monitor for the guard.

#include <Arduino.h>
#include <SoftwareSerial.h>

#define SW_RX 25
#define SW_TX 26

SoftwareSerial swSerial(SW_RX, SW_TX);

void setup() {
  Serial.begin(115200);
  swSerial.begin(9600);

  Serial.println("Node B - Inside facility");
  Serial.println("Waiting for security updates...");
}

unsigned long lastHeartbeat = 0;

void loop() {
  if (swSerial.available()) {
    String message = swSerial.readStringUntil('\n');
    message.trim();
    if (message.length() > 0) {
      Serial.print("Security status: ");
      Serial.println(message);
    }
  }

  // Heartbeat every 10s
  // if (millis() - lastHeartbeat > 10000) {
  //   lastHeartbeat = millis();
  //   Serial.println("[heartbeat] Node B listening...");
  // }
}
