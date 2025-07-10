#include <Arduino.h>
#include "config.h"
#include "model/data_structs.h"
#include "sensors/pir_sensor.h"
#include "debug/debug_macros.h"

static bool lastMotion = false;

unsigned long lastPirError = 0;

extern bool pirTriggered;
extern unsigned long lastPirTrigger;
extern bool ledBlink;
extern unsigned long ledBlinkStart;

void setupPIR()
{
    pinMode(PIR_PIN, INPUT);
    SENSOR_DEBUG_PRINTF("PIR sensor initialized on pin %d\n", PIR_PIN);
    ESP.wdtFeed();
}

void IRAM_ATTR handlePirInterrupt()
{
    // Skip if in sensorless mode or PIR not available
    if (config.sensorless_mode || !sensorData.pir_available)
    {
        return;
    }

    if (!pirTriggered)
    {
        pirTriggered = true;
        sensorData.motion = true;
        lastPirTrigger = millis();

        // Set LED blink flag (will be handled in main loop)
        ledBlink = true;
        ledBlinkStart = millis();
        DEBUG_PRINTLN("Motion detected!");
    }
}

void pirInit()
{
    pinMode(PIR_PIN, INPUT);
}

void readPIR()
{
    int val = digitalRead(PIR_PIN);
    sensorData.motion = (val == HIGH);
    sensorData.pir_available = true; // Assume always available unless you want to add error logic
    if (sensorData.motion != lastMotion)
    {
        lastMotion = sensorData.motion;
        sensorData.pir_error_count = 0;
    }
}
void updatePirInterrupt()
{
    static bool interruptAttached = false;

    // Only manage interrupt if not in sensorless mode
    if (config.sensorless_mode)
    {
        if (interruptAttached)
        {
            detachInterrupt(digitalPinToInterrupt(PIR_PIN));
            interruptAttached = false;
            DEBUG_PRINTLN("PIR interrupt detached (sensorless mode)");
        }
        return;
    }

    // Attach interrupt if PIR is available and not already attached
    if (sensorData.pir_available && !interruptAttached)
    {
        attachInterrupt(digitalPinToInterrupt(PIR_PIN), handlePirInterrupt, RISING);
        interruptAttached = true;
        DEBUG_PRINTLN("PIR interrupt attached");
    }
    // Detach interrupt if PIR is not available and currently attached
    else if (!sensorData.pir_available && interruptAttached)
    {
        detachInterrupt(digitalPinToInterrupt(PIR_PIN));
        interruptAttached = false;
        DEBUG_PRINTLN("PIR interrupt detached (sensor not available)");
    }
}