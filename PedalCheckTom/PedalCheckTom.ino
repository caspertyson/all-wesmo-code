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

int comparePedalSensors()
{
  p1Val = analogRead(p1);
  p2Val = analogRead(p2);

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
  difference = abs(p1Percentage - p2Percentage);
  signalOut = (p1Percentage + p2Percentage) / 2;
  Serial.printf("Signal: %f  Diff: %f\n", signalOut, difference);
  if (difference > 0.1)
  {
      signalOut = 0;
  }
  return signalOut;

}
