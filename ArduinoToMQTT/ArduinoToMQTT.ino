#include <Ethernet.h>
#include <MQTT.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {192, 168, 2, 2};  // <- change to match your network

EthernetClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("connecting...");
  while (!client.connect("arduino")) {  // Assuming no username/password for wesmo.co.nz
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
}


void setup() {
  Serial.begin(115200);
  Ethernet.begin(mac, ip);
  client.begin("wesmo.co.nz", 1883, net);  // Changed the broker details
  connect();
}

void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }
  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("newTopic", "world");  // Changed the publish topic
  }
}
