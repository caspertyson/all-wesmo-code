#include <Ethernet.h>
#include <MQTT.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {192, 168, 2, 2};  // <- change to match your network

EthernetClient net;
MQTTClient client;

#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(9);                               // Set CS to pin 10

void connect() {
  Serial.print("connecting...");
  while (!client.connect("arduino")) {  // Assuming no username/password for wesmo.co.nz
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
}

void setup()
{
  Serial.begin(115200);
  // Ethernet.localIP();
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
  
  Serial.println("finished CAN setup...");
  
  Serial.println("Start ethernet connection:");
  Ethernet.begin(mac, ip);
  client.begin("52.64.104.47", 1883, net);  // Changed the broker details
  connect();
  Serial.println("connected");
}
unsigned long lastSendTime = 0;  // Last send time for identifier 0x06000009
int sendCount = 0;  // Count of the sent messages for identifier 0x06000009

void loop()
{
  client.loop();
  if (!client.connected()) {
    connect();
  }

  if(millis() - lastSendTime >= 1000) {
    sendCount = 0;
    lastSendTime = millis();
  }

  if (!digitalRead(CAN0_INT))  // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);  // Read data: len = data length, buf = data byte(s)
    if ((rxId == 0x86000009 && sendCount >= 1) || rxId == 0x85800009) {
      return;
    }
    char fullMessage[100]; // assuming this length is sufficient for the message + data bytes
    if ((rxId & 0x80000000) == 0x80000000)  // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(fullMessage, "%.8lX ", (rxId & 0x1FFFFFFF));
    else
      sprintf(fullMessage, "%.3lX ", rxId);

    char tempString[10];  // For byte values
    for (byte i = 0; i < len; i++) {
      sprintf(tempString, "%.2X ", rxBuf[i]);
      strcat(fullMessage, tempString); // append byte value to full message
    }

    // Remove last space character
    int lastCharPos = strlen(fullMessage) - 1;
    if (fullMessage[lastCharPos] == ' ') {
      fullMessage[lastCharPos] = '\0';
    }
    if (rxId == 0x86000009) {
      sendCount++; 
    }
    client.publish("newTopic", String(fullMessage).c_str()); 
    Serial.println(fullMessage);
  }
  // String message = "hello";
  // client.publish("newTopic", String(message).c_str()); 

}
