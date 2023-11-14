#include <Arduino.h>
#include <stdio.h>

// Potentiometer is connected to GPIO 34 (Analog ADC2_CH3)
const int p1 = 2;
const int p2 = 15;

// variable for storing the potentiometer value
int p1Val = 0;
int p2Val = 0;

// Max and Minvalues for each sensor
int p1Max = 2000;
int p2Max = 2000;
int p1Min = 600;
int p2Min = 600;

double p1Percentage = 0;
double p2Percentage = 0;
double difference = 0;
double signalOut = 0;

// the number of the LED pin
const int ledPin = 16; // 16 corresponds to GPIO16

// PWM Transmission setup
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;


void comparePedalSensors()
{
    if (p1Val > p1Max)
        p1Max = p1Val;
    if (p2Val > p2Max)
        p2Max = p2Val;
    if (p1Val < p1Min)
        p1Min = p1Val;
    if (p2Val < p2Min)
        p2Min = p2Val;

    p1Percentage = (double)(p1Val - p1Min) / (p1Max - p1Min);
    p2Percentage = (double)(p2Val - p2Min) / (p2Max - p2Min);
    // Serial.printf("1: %f  2: %f\n", p1Percentage, p2Percentage);
    // Serial.printf("Value: %d  Min: %d  Max: %d  Per: %f\n", p1Val, p1Min, p1Max, p1Percentage);
    difference = abs(p1Percentage - p2Percentage);
    signalOut = (p1Percentage + p2Percentage) / 2;
    Serial.printf("Signal: %f  Diff: %f\n", signalOut, difference);
    if (difference > 0.1)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.printf("Sensor values too different\n");
    }
    else
    {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.printf("Sensor values agree\n");
    }
}
void sendSignal()
{ 
    // max Duty Cycle is 255 and min is 0
    int dutyCycle = 255 * signalOut;
    ledcWrite(ledChannel, dutyCycle);
    // Serial.printf("Duty Cycle Output: %d\n", dutyCycle);
}

void setupPedalCheck()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // configure LED PWM functionalitites
    ledcSetup(ledChannel, freq, resolution);

    // attach the channel to the GPIO to be controlled
    ledcAttachPin(ledPin, ledChannel);
}

void loopPedalCheck()
{
    // Reading potentiometer value
    p1Val = analogRead(p1);
    p2Val = analogRead(p2);
    comparePedalSensors();
    sendSignal();
    Serial.printf("\n");
    delay(1000);
}