#include <Arduino.h>
#include <stdio.h>
#include "errors.h"

// Potentiometer is connected to GPIO 34 (Analog ADC2_CH3)
const int p1 = 32;
const int p2 = 33;

// variable for storing the potentiometer value
int p1Val = 0;
int p2Val = 0;

// Max and Minvalues for each sensor
int p1Max = 4095;
int p2Max = 3289;
int p1Min = 1550;
int p2Min = 1750;

const float alpha = 0.4; // Smoothing factor (adjust as needed)
int filteredP1Val = 0;
int filteredP2Val = 0;

double p1Percentage = 0;
double p2Percentage = 0;
double difference = 0;
double signalOut = 0;

unsigned long startTime = 0; // Variable to store the start time of the difference condition
bool differenceExceeded = false; // Flag to keep track of the difference condition

double comparePedalSensors(byte &currentError, byte ERROR_PEDAL_SENSORS_DISAGREE)
{
  p1Val = analogRead(p1);
  p2Val = analogRead(p2);

    // smoother 
  filteredP1Val = alpha * p1Val + (1 - alpha) * filteredP1Val;
  filteredP2Val = alpha * p2Val + (1 - alpha) * filteredP2Val;

  p1Percentage = (double)(filteredP1Val - p1Min) / (p1Max - p1Min);
  p2Percentage = (double)(filteredP2Val - p2Min) / (p2Max - p2Min);
  if(p1Percentage < 0)
    p1Percentage = 0;
  if(p2Percentage < 0)
    p2Percentage = 0;
  difference = abs(p1Percentage - p2Percentage);
  
  signalOut = (p1Percentage + p2Percentage) / 2;
  // signalOut = p2Percentage; 
  
  // Serial.printf("Signal: %f  Diff: %f\n", signalOut, difference);

  // Serial.println(String(p1Val) + "," + String(p2Val));
  // Serial.println(String(filteredP1Val) + "," + String(filteredP2Val));

  // if (difference > 0.1)
  // {
  //   if (!differenceExceeded)
  //   {
  //     // First time difference is > 10%
  //     startTime = millis();
  //     differenceExceeded = true;
  //   }
  //   else if (millis() - startTime >= 100)
  //   {
  //     signalOut = 0;
  //     setError(currentError, ERROR_PEDAL_SENSORS_DISAGREE);
  //   }
  // }
  // else
  // {
  //   differenceExceeded = false;
  //   clearError(currentError, ERROR_PEDAL_SENSORS_DISAGREE);
  // }
  return signalOut;
}