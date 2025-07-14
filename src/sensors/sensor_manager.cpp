#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "sensors/sensors.h"
#include "debug/debug_macros.h"
#include "comm/wifi_manager.h"
#include "sensors/sensor_manager.h"
#include "model/data_structs.h"
#include "debug/debug_macros.h"

void setupAllSensors()
{
    if (config.use_dht)
    {
        setupDHT();
    }
    if (config.use_tsl2561)
    {
        setupTSL2561();
    }
    if (config.use_pir)
    {
        setupPIR();
    }
    if (config.use_ld2410)
    {
        setupLD2410();
    }
    // Initial sensor read and availability check
    SENSOR_DEBUG_PRINTLN("Performing initial sensor read...");
    readAllSensors();
    printSensorData();

    ESP.wdtFeed();
}

void readAllSensors()
{
    if (config.use_dht)
    {
        readDHT();
    }
    if (config.use_tsl2561)
    {
        readTSL2561();
    }
    if (config.use_pir)
    {
        readPIR();
    }
    if (config.use_ld2410)
    {
        readLD2410();
    }
}
void printInitialSensorData()
{
    // Print initial sensor status
    SENSOR_DEBUG_PRINTLN("=== Initial Sensor Status ===");
    if (config.use_dht)
    {
        SENSOR_DEBUG_PRINTF("DHT11 (Temp/Humidity): %s\n", sensorData.dht_available ? "Available" : "Not Available");
    }
    if (config.use_tsl2561)
    {
        SENSOR_DEBUG_PRINTF("TSL2561 (Light): %s (requires manual reset if connected)\n", sensorData.tsl_available ? "Available" : "Not Available");
    }
    if (config.use_ld2410)
    {
        SENSOR_DEBUG_PRINTF("LD2410 (Motion): %s (requires manual reset if connected)\n", sensorData.radar_available ? "Available" : "Not Available");
    }

    if (config.use_pir)
    {
        SENSOR_DEBUG_PRINTF("PIR (Motion): %s (requires manual reset if connected)\n", sensorData.pir_available ? "Available" : "Not Available");
    }

    SENSOR_DEBUG_PRINTLN("=============================");
}
void printSensorData()
{
    SENSOR_DEBUG_PRINTF("=== Sensor Data === [%s] ===\n", deviceHostname.c_str());
    if (config.use_dht)
    {
        SENSOR_DEBUG_PRINTF("Temperature: %.2f°C %s\n", sensorData.temperature, sensorData.dht_available ? "" : "(DHT11 not available)");
        SENSOR_DEBUG_PRINTF("Humidity: %.2f%% %s\n", sensorData.humidity, sensorData.dht_available ? "" : "(DHT11 not available)");
    }
    if (config.use_tsl2561)
    {
        SENSOR_DEBUG_PRINTF("Luminescence: %.2f lux %s\n", sensorData.lux, sensorData.tsl_available ? "" : "(TSL2561 not available)");
    }
    if (config.use_ld2410)
    {
        SENSOR_DEBUG_PRINTF("Radar Presence: %s\n", sensorData.radar_presence ? "YES" : "NO");
        SENSOR_DEBUG_PRINTLN("Motion Source: Radar (LD2410)");
    }

    if (config.use_pir)
    {
        SENSOR_DEBUG_PRINTF("Motion: %s\n", sensorData.motion ? "YES" : "NO");
        SENSOR_DEBUG_PRINTLN("Motion Source: PIR");
    }

    MEMORY_DEBUG_PRINTF("Free Heap: %d bytes\n", ESP.getFreeHeap());
    SENSOR_DEBUG_PRINTLN("==================");
}
