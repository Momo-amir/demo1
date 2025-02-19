#ifndef FACTORY_RESET_H
#define FACTORY_RESET_H

#include <Arduino.h>

void checkFactoryReset();  // Call this in loop()
void factoryReset();       // Handles the actual reset process

#endif