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
#if USE_DHT
    setupDHT();
#endif
#if USE_TSL2561
    setupTSL2561();
#endif
#if USE_PIR
    setupPIR();
#endif
#if USE_LD2410
    setupLD2410();
#endif
    // Initial sensor read and availability check
    SENSOR_DEBUG_PRINTLN("Performing initial sensor read...");
    readAllSensors();
    printSensorData();

    ESP.wdtFeed();
}

void readAllSensors()
{
#if USE_DHT
    readDHT();
#endif
#if USE_TSL2561
    readTSL2561();
#endif
#if USE_PIR
    readPIR();
#endif
#if USE_LD2410
    readLD2410();
#endif
}
void printInitialSensorData()
{
    // Print initial sensor status
    SENSOR_DEBUG_PRINTLN("=== Initial Sensor Status ===");
#if USE_DHT
    SENSOR_DEBUG_PRINTF("DHT11 (Temp/Humidity): %s\n", dhtAvailable ? "Available" : "Not Available");
#endif
#if USE_TSL2561
    SENSOR_DEBUG_PRINTF("TSL2561 (Light): %s (requires manual reset if connected)\n", tslAvailable ? "Available" : "Not Available");
#endif
    if (config.use_radar)
    {
#if USE_LD2410
        SENSOR_DEBUG_PRINTF("LD2410 (Motion): %s (requires manual reset if connected)\n", sensorData.radar_available ? "Available" : "Not Available");
#endif
    }
    else
    {
#if USE_PIR
        SENSOR_DEBUG_PRINTF("PIR (Motion): %s (requires manual reset if connected)\n", sensorData.pir_available ? "Available" : "Not Available");
#endif
    }
    SENSOR_DEBUG_PRINTLN("=============================");
}
void printSensorData()
{
    SENSOR_DEBUG_PRINTF("=== Sensor Data === [%s] ===\n", deviceHostname.c_str());
#if USE_DHT
    SENSOR_DEBUG_PRINTF("Temperature: %.2f°C %s\n", sensorData.temperature, dhtAvailable ? "" : "(DHT11 not available)");
    SENSOR_DEBUG_PRINTF("Humidity: %.2f%% %s\n", sensorData.humidity, dhtAvailable ? "" : "(DHT11 not available)");
#endif
#if USE_TSL2561
    SENSOR_DEBUG_PRINTF("Luminescence: %.2f lux %s\n", sensorData.lux, tslAvailable ? "" : "(TSL2561 not available)");
#endif
    if (config.use_radar)
    {
#if USE_LD2410
        SENSOR_DEBUG_PRINTF("Radar Presence: %s\n", sensorData.radar_presence ? "YES" : "NO");
        SENSOR_DEBUG_PRINTLN("Motion Source: Radar (LD2410)");
#endif
    }
    else
    {
#if USE_PIR
        SENSOR_DEBUG_PRINTF("Motion: %s\n", sensorData.motion ? "YES" : "NO");
        SENSOR_DEBUG_PRINTLN("Motion Source: PIR");
#endif
    }
    MEMORY_DEBUG_PRINTF("Free Heap: %d bytes\n", ESP.getFreeHeap());
    SENSOR_DEBUG_PRINTLN("==================");
}
