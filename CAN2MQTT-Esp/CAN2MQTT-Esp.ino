#include <driver/can.h>
#include "MQTT.h"
#define TAG "CAN_RECEIVE"

// MQTTClient mqttClient("iPhone", "caspertyson", "172.20.10.14", 1883, "espMothuFker");
MQTTClient mqttClient("Caspers iPhone", "caspertyson", "wesmo.co.nz", 1883, "espMothuFker");

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup reached");

  mqttClient.connectWiFi();
  Serial.println("wifi connected");

  mqttClient.connectMQTTBroker();

  // Initialize configuration structures using macro initializers
  can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, CAN_MODE_NORMAL);
  can_timing_config_t t_config = CAN_TIMING_CONFIG_250KBITS();
  can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

  // Install CAN driver
  if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return;
  }

  // Start CAN driver
  if (can_start() == ESP_OK) {
    Serial.println("Driver started");
  } else {
    Serial.println("Failed to start driver");
    return;
  }
}

unsigned long lastSendTime = 0;  // Last send time for identifier 0x06000009
int sendCount = 0;  // Count of the sent messages for identifier 0x06000009

void loop() {
  // send speed request 10 x a second
  if(millis() - lastSendTime >= 1000) {
    sendCount = 0;
    lastSendTime = millis();
  }
  
  can_message_t message;
  esp_err_t ret = can_receive(&message, pdMS_TO_TICKS(1000));
  if (ret == ESP_OK) {
    if ((message.identifier == 0x06000009 && sendCount >= 1) || message.identifier == 0x05800009) {
      return;
    }
    char str[64]; // Buffer to store the message
    int len = sprintf(str, "%04x ", message.identifier); // Print the message ID
    // Print each data byte
    for (int i = 0; i < message.data_length_code; i++) {
      len += sprintf(str + len, "%02x ", message.data[i]);
    }
    mqttClient.publishMessage("newTopic", String(str).c_str());
    if (message.identifier == 0x06000009) {
      sendCount++; 
    }
    Serial.println(String(str));
  } else if (ret == ESP_ERR_TIMEOUT) {
    Serial.println("Timeout - no message received");
  } else {
    Serial.println("Failed to receive message");
  }
}
