#include <Arduino.h>
#include <stdio.h>

//&errorByte passes pointer to errorByte, not just the value of errorByte
void setError(byte &currentError, byte errorFlag) {
  currentError |= errorFlag;
}

// AND NOT errorFlag clears the error
void clearError(byte &currentError, byte errorFlag) {
  currentError &= ~errorFlag;
}
