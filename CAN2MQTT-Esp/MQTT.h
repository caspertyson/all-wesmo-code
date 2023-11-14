#include <WiFi.h>
#include <PubSubClient.h>

class MQTTClient {
private:
  const char* ssid;
  const char* password;
  const char* mqttBroker;
  const int mqttPort;
  const char* mqttClientID;
  WiFiClient espClient;
  PubSubClient client;

public:
  MQTTClient(const char* _ssid, const char* _password, const char* _mqttBroker, int _mqttPort, const char* _mqttClientID)
    : ssid(_ssid), password(_password), mqttBroker(_mqttBroker), mqttPort(_mqttPort), mqttClientID(_mqttClientID),
      client(espClient)
  {
  }

  void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Connected to Wi-Fi");
  }

  void connectMQTTBroker() {
    client.setServer(mqttBroker, mqttPort);
    while (!client.connected()) {
      if (client.connect(mqttClientID)) {
        // Serial.println("Connected to MQTT broker");
      } else {
        // Serial.print("Failed to connect to MQTT broker, rc=");
        // Serial.print(client.state());
        // Serial.println(" Retrying in 5 seconds...");
        delay(5000);
      }
    }
  }

  void publishMessage(const char* topic, const char* message) {
    if (client.connected()) {
      client.publish(topic, message);
    } else {
      // Serial.println("Not connected to MQTT broker");
    }
  }

  void loop() {
    client.loop();
  }
};
