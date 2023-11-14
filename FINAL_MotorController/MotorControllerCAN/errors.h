#ifndef MYLIBRARY_H
#define MYLIBRARY_H

#include <Arduino.h>

void setError(byte &currentError, byte errorFlag);
void clearError(byte &currentError, byte errorFlag);

#endif
