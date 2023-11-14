// #include "pedalCheck.h"
#include <CAN.h>

#define RelayPin1 19
#define RelayPin2 18
#define SafetyCircuitPin 2
#define TX_GPIO_NUM 5  // Connects to CTX
#define RX_GPIO_NUM 4  // Connects to CRX

enum class State {
  Off,
  Precharge,
  Running
};

// volatile int startPrecharge;
// volatile State current_state = State::Off;
// volatile bool firstRelayOff = true;
int startPrecharge;
State current_state = State::Off;
bool firstRelayOff = true;
bool canIsEnabled = false;

bool readyToPrecharge = false;
bool enableMotorController = true;

// void SafetyCircuitInterrupt() {
//   current_state = State::Off;
//   firstRelayOff = true;
//   // Serial.print("The State is now off");
//   // Kill Motor? BMS? Other?
// }
// int canSender() {
//   // send packet: id is 11 bits, packet can contain up to 8 bytes of data
//   // Serial.print("Sending packet ... ");
//   //CAN.beginPacket (0x01);  //sets the ID and clears the transmit buffer
//   CAN.beginExtendedPacket(0x06000009);
//   CAN.write(0x23);  //write data to buffer. data is not sent until endPacket() is called.
//   CAN.write(0x00);
//   CAN.write(0x20);
//   CAN.write(0x01);
//   CAN.write(0x00);
//   CAN.write(0x00);
//   CAN.write(0x13);
//   CAN.write(0x88);
//   int success = CAN.endPacket();

//   //RTR packet with a requested data length
//   // CAN.beginPacket(0x01, 8, false);
//   // CAN.endPacket();
//   // Serial.println("Send speed request");
//   delay(50);
//   return success;
// }
int canSendEnable() {
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending packet ... ");
  CAN.beginPacket (0x01);  //sets the ID and clears the transmit buffer
  // CAN.beginExtendedPacket(0x06000009);
  CAN.write(0x23);  //write data to buffer. data is not sent until endPacket() is called.
  CAN.write(0x0D);
  CAN.write(0x20);
  CAN.write(0x01);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  CAN.write(0x00);
  int success = CAN.endPacket();

  //RTR packet with a requested data length
  // CAN.beginExtendedPacket(0x06000009, 8, false);
  // CAN.endPacket();
  Serial.println("CAN Enable");

  delay(50);
  return success;
}

void setup() {
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  pinMode(SafetyCircuitPin, INPUT_PULLDOWN);
  // attachInterrupt(digitalPinToInterrupt(SafetyCircuitPin), SafetyCircuitInterrupt, FALLING);
  Serial.begin(115200);
  // Can Setup
  while (!Serial);
  delay(1000);
  Serial.println("CAN Receiver/Receiver");
  CAN.setPins(RX_GPIO_NUM, TX_GPIO_NUM);
  // CAN.loopback();
  if (!CAN.begin(250E3)) {  // start the CAN bus at 250 kbps - 500E3 for 500kbps
    Serial.println("Starting CAN failed!");
    while (1);
  } else {
    Serial.println("CAN Initialized");
  }
  // CAN.setPins(TX_GPIO_NUM, RX_GPIO_NUM);

  // if (!CAN.begin(250E3)) {  // start the CAN bus at 500 kbps
  //   Serial.println("Starting CAN failed!");
  //   while (1);
  // }

}

void loop() {
  bool SafetyOn = digitalRead(SafetyCircuitPin);
  if (!SafetyOn) {
    current_state = State::Off;
  }

  switch (current_state) {
    case State::Off:
      // Serial.println("Mode: off");

      digitalWrite(RelayPin1, HIGH);
      digitalWrite(RelayPin2, HIGH);

      if (SafetyOn) {
        startPrecharge = millis();
        digitalWrite(RelayPin1, LOW);
        current_state = State::Precharge;
      } 
      break;

    case State::Precharge:
      // Serial.println("Mode: precharge ");
      if (millis() - startPrecharge > 4000) {
        // Serial.println("Time: " + String(millis() - startPrecharge));
        digitalWrite(RelayPin1, HIGH);
        // canSendEnable();
        current_state = State::Running;
      }
      else if (millis() - startPrecharge > 2000) {
        // Serial.println("Time: " + String(millis() - startPrecharge));
        digitalWrite(RelayPin2, LOW);
      }
      break;

    case State::Running:
      // loopPedalCheck();
      // if (enableMotorController) {
      //   canSendEnable();
      //   enableMotorController = false;
      // }
      // send 750 rpm request every 50ms
      // canSender();

      if(canSendEnable()){
        Serial.println("Successfully Sent Enable");
      }
      break;
  }
}
