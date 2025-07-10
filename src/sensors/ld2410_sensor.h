#pragma once
#include <SoftwareSerial.h>
#include <MyLD2410.h>
extern SoftwareSerial ld2410Serial;
extern MyLD2410 radar;
extern bool ledBlink;
extern long ledBlinkStart;

void setupLD2410();
void readLD2410();