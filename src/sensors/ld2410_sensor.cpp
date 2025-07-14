#include <SoftwareSerial.h>
#include <MyLD2410.h>
#include "config.h"
#include "debug/debug_macros.h"
#include "model/data_structs.h"
#include "sensors/ld2410_sensor.h"

SoftwareSerial ld2410Serial(LD2410_RX_PIN, LD2410_TX_PIN);
MyLD2410 radar(ld2410Serial);

void setupLD2410()
{
    delay(2000);
    ld2410Serial.begin(LD2410_BAUD_RATE_2);
    radar.begin();
    sensorData.radar_available = true;
}

void readLD2410()
{
    if (radar.check() == MyLD2410::Response::DATA)
    {
        sensorData.radar_presence = radar.presenceDetected();
        sensorData.presence = sensorData.radar_presence;
        sensorData.radar_available = true;
        if (sensorData.radar_presence)
        {
            SENSOR_DEBUG_PRINTLN("LD2410C: Presence detected!");
            ledBlink = true;
            ledBlinkStart = millis();
            DEBUG_PRINTLN("Motion detected!");
        }
        else
        {
            SENSOR_DEBUG_PRINTLN("LD2410C: No presence.");
        }
    }
    else
    {
        sensorData.radar_presence = false;
        sensorData.radar_available = false;
    }
}