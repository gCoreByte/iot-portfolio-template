#include <Arduino.h>
#include <ESP8266WiFi.h>


const char* ssid = "IOT44";
const char* password = "iotempire";


volatile bool interruptFlag = false;


unsigned long counter = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;


void ICACHE_RAM_ATTR handleInterrupt() {
 interruptFlag = true;
}


void setup() {
 Serial.begin(115200);
 pinMode(D3, INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(D3), handleInterrupt, FALLING);


 WiFi.begin(ssid, password);
 Serial.print("Connecting to WiFi");


 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }


 Serial.println("\nConnected to WiFi");
 Serial.print("IP Address: ");
 Serial.println(WiFi.localIP());
}


void loop() {
 if (interruptFlag) {
   unsigned long now = millis();
   if (now - lastDebounceTime > debounceDelay) {
     lastDebounceTime = now;
     Serial.println("Interrupt occurred!");
   }
   interruptFlag = false;
 }


 // Simulate some work in the main loop
 delay(100);
 Serial.print("Hello, I am alive and counting: ");
 Serial.println(counter);
 counter ++;
}
