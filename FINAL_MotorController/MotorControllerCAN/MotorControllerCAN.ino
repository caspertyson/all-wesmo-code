#include "driver/gpio.h"
#include "driver/can.h"
#include <Arduino.h>
#include <stdio.h>
#include "pedalCheck.h"
#include "errors.h"

#define TX_GPIO_NUM 5  // Connects to CTX
#define RX_GPIO_NUM 4  // Connects to CRX
#define RelayPin1 19
#define RelayPin2 18
#define SafetyCircuitPin 2

enum class State {
  Off,
  Precharge1,
  Precharge2,
  Running
};

int startPrecharge;
double accelPedal = 0.0;
int DriveSignal = 0;
State current_state = State::Off;
bool firstRelayOff = true;
bool canIsEnabled = false;
bool readyToPrecharge = false;
bool enableMotorController = true;

const byte ERROR_PEDAL_SENSORS_DISAGREE = 0x01;  // Binary: 00000001
const byte ERROR_SAFETY_CIRCUIT_TRIPPED = 0x02;  // Binary: 00000010
const byte ERROR_CAN_FAILED_TO_SEND = 0x04;  // Binary: 00000100
const byte ERROR_LOOP_TIME_OVER_5MS = 0x08;  // Binary: 00001000
const byte ERROR_IMD_ERROR = 0x10;  // Binary: 00010000

unsigned long currentTime = 0;
unsigned long lastTime = 0;

uint8_t enableMotor[8] = {0x23, 0x0D, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00};
uint8_t disableMotor[8] = {0x23, 0x0C, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00};
uint8_t setMotorSpeed[8] = {0x23, 0x00, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00};
uint8_t zeroMessage[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

byte currentError = 0;
byte lastError = 0;

can_message_t createMessage(uint32_t id, uint8_t data[8]) {
  can_message_t message;
  message.identifier = id;
  message.flags = CAN_MSG_FLAG_EXTD;
  message.data_length_code = 8;
  for(int i = 0; i < 8; i++) {
    message.data[i] = data[i];
  }
  return message;
}

void setup() {
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  pinMode(SafetyCircuitPin, INPUT_PULLDOWN);
  // attachInterrupt(digitalPinToInterrupt(SafetyCircuitPin), SafetyCircuitInterrupt, FALLING);
  Serial.begin(115200);

  //Initialize configuration structures using macro initializers
  can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, CAN_MODE_NORMAL);
  can_timing_config_t t_config = CAN_TIMING_CONFIG_250KBITS();
  can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

  //Install CAN driver
  if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
      printf("Driver installed\n");
  } else {
      printf("Failed to install driver\n");
      return;
  }

  //Start CAN driver
  if (can_start() == ESP_OK) {
      printf("Driver started\n");
  } else {
      printf("Failed to start driver\n");
      return;
  }
}

/////////////////////////// MAIN LOOP ///////////////////////////

void loop() {
  can_message_t message;
  esp_err_t result = can_receive(&message, pdMS_TO_TICKS(50)); // Wait for a message for up to 50ms

  if(result == ESP_OK) {
      Serial.print("ID: 0x");
      Serial.print(message.identifier, HEX);
      Serial.print(" Data: ");
      for (int i = 0; i < message.data_length_code; i++) {
          Serial.print(message.data[i], HEX);
          Serial.print(" ");
      }
      Serial.println();
  }else{
    Serial.println("No CAN message");
  }





  currentTime = millis();
  if((currentTime - lastTime) > 1000){
    // Transmit State of VCU
    can_message_t message = createMessage(0x06000008,zeroMessage);
    message.data[7] = int(current_state);
    if (!can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
      // printf("Failed to queue disable motor for transmission\n");
    } 
    lastTime = millis();
  }

  unsigned long startTime = micros();  // Record the start time

  bool SafetyOn = digitalRead(SafetyCircuitPin);

  if (!SafetyOn) {
    setError(currentError, ERROR_SAFETY_CIRCUIT_TRIPPED);

    can_message_t message = createMessage(0x06000009,disableMotor);
    if (!can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
      // printf("Failed to queue disable motor for transmission\n");
    }
    current_state = State::Off;
  }else{
    clearError(currentError, ERROR_SAFETY_CIRCUIT_TRIPPED);
  }
  
  ////////////////////////////// STATE MACHINE ////////////////////////////
  switch (current_state) {
    printf("running \n");
    case State::Off:
      digitalWrite(RelayPin1, HIGH);
      digitalWrite(RelayPin2, HIGH);

      if (SafetyOn) {
        startPrecharge = millis();
        digitalWrite(RelayPin1, LOW);
        current_state = State::Precharge1;
      }
      break;

    case State::Precharge1:
      if (millis() - startPrecharge > 2000) {
        digitalWrite(RelayPin2, LOW);
        current_state = State::Precharge2;
      }
      break;

    case State::Precharge2:
      if (millis() - startPrecharge > 4000) {
        digitalWrite(RelayPin1, HIGH);

        can_message_t message = createMessage(0x06000009,enableMotor);
        if (!can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
          // printf("Failed to queue enable message \n");
          // current_state = State::Off;
        } 
        current_state = State::Running;
      }
      break;

    case State::Running:
      can_message_t message = createMessage(0x06000009,setMotorSpeed);

      accelPedal = comparePedalSensors(currentError, ERROR_PEDAL_SENSORS_DISAGREE);
      accelPedal = accelPedal * 100;
      DriveSignal = map(accelPedal, 0, 100, 0, 10000);
      // Serial.println("accel pedal pos: " + String(DriveSignal));

      // accelPedal = analogRead(15); // read throttle 0-4095
      // DriveSignal = map(accelPedal, 0, 4095, 0, 10000);
      // Serial.println("Accel Pedal: " + String(accelPedal));
      // Serial.println("Drive Signal: " + String(DriveSignal));

      message.data[4] = DriveSignal >> 24;
      message.data[5] = DriveSignal >> 16;
      message.data[6] = DriveSignal >> 8;
      message.data[7] = DriveSignal;
      
      if (!can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
        // printf("Failed to queue speed message \n");
        // current_state = State::Off;
        setError(currentError, ERROR_CAN_FAILED_TO_SEND);
        
        can_message_t message = createMessage(0x06000009,enableMotor);
        if (!can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
          // printf("Failed to queue enable message in running mode \n");
          // current_state = State::Off;
        }
      }else{
        clearError(currentError, ERROR_CAN_FAILED_TO_SEND);
      }
      break;
  }
  unsigned long endTime = micros();
  unsigned long elapsedTime = endTime - startTime;
  
  if(currentError != lastError){
    can_message_t message = createMessage(0x06000007,zeroMessage);
    message.data[7] = currentError;
    if (!can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
      // printf("Failed to queue disable motor for transmission\n");
    }
    lastError = currentError;
  }
}