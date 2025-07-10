#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "config.h"
#include "model/data_structs.h"
#include "sensors/tsl2561_sensor.h"
#include "debug/debug_macros.h"

extern long currentTime;
Adafruit_TSL2561_Unified tsl(TSL2561_ADDR_FLOAT, 12345);
bool tslAvailable = false;
int tslErrorCount = 0;
unsigned long lastTslError = 0;

void setupTSL2561()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    SENSOR_DEBUG_PRINTF("I2C initialized on SDA:%d, SCL:%d\n", SDA_PIN, SCL_PIN);
    ESP.wdtFeed();
    // Initialize TSL2561 light sensor
    SENSOR_DEBUG_PRINTLN("Initializing TSL2561 light sensor...");
    if (tsl.begin())
    {
        SENSOR_DEBUG_PRINTLN("TSL2561 light sensor initialized");

        // Configure TSL2561
        tsl.enableAutoRange(true);
        tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);

        // Get sensor details
        sensor_t sensor;
        tsl.getSensor(&sensor);
        SENSOR_DEBUG_PRINTF("TSL2561 Sensor: %s\n", sensor.name);
        SENSOR_DEBUG_PRINTF("Driver Version: %d\n", sensor.version);
        SENSOR_DEBUG_PRINTF("Unique ID: %d\n", sensor.sensor_id);
        SENSOR_DEBUG_PRINTF("Max Value: %.1f lux\n", sensor.max_value);
        SENSOR_DEBUG_PRINTF("Min Value: %.1f lux\n", sensor.min_value);
        SENSOR_DEBUG_PRINTF("Resolution: %.1f lux\n", sensor.resolution);

        // Note: TSL2561 will be marked available only after successful reads
    }
    else
    {
        SENSOR_DEBUG_PRINTLN("TSL2561 light sensor not found! Check wiring.");
    }
}

void readTSL2561()
{
    sensors_event_t event;
    tsl.getEvent(&event);

    if (event.light)
    {
        sensorData.lux = event.light;
        // Reset error counter on successful read
        if (tslErrorCount > 0)
        {
            SENSOR_DEBUG_PRINTF("TSL2561 working again. Error count reset from %d\n", tslErrorCount);
            tslErrorCount = 0;
        }
    }
    else
    {
        SENSOR_DEBUG_PRINTLN("TSL2561 sensor read failed");
        tslErrorCount++;
        lastTslError = currentTime;

        if (tslErrorCount >= SENSOR_ERROR_THRESHOLD)
        {
            SENSOR_DEBUG_PRINTF("TSL2561 sensor failed %d times. Marking as unavailable.\n", tslErrorCount);
            tslAvailable = false;
            sensorData.lux = 0.0;
        }
    }
}