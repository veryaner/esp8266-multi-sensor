#pragma once
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
extern int tslErrorCount;
extern unsigned long lastTslError;
extern Adafruit_TSL2561_Unified tsl;
void setupTSL2561();
void readTSL2561();