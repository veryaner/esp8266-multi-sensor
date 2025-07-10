#include "config_manager.h"
#include "config.h"
#include <EEPROM.h>
#include <Arduino.h>
#include "debug/debug_macros.h"
#include "data_structs.h"

void loadConfig()
{
    EEPROM.get(0, config);
    if (config.mqtt_port == 0 || strlen(config.mqtt_broker) == 0)
    {
        DEBUG_PRINTLN("Loading default configuration...");
        strcpy(config.mqtt_broker, MQTT_BROKER);
        config.mqtt_port = MQTT_PORT;
        strcpy(config.mqtt_username, MQTT_USERNAME);
        strcpy(config.mqtt_password, MQTT_PASSWORD);
        strcpy(config.location, "sensors");
        config.mqtt_enabled = ENABLE_MQTT;
        config.sensorless_mode = DEFAULT_SENSORLESS_MODE;
        config.use_radar = DEFAULT_USE_RADAR;
        saveConfig();
    }
    config.sensorless_mode = DEFAULT_SENSORLESS_MODE;
    DEBUG_PRINTLN("Configuration loaded:");
    DEBUG_PRINTF("  MQTT Broker: %s:%d\n", config.mqtt_broker, config.mqtt_port);
    DEBUG_PRINTF("  Location: %s\n", config.location);
    DEBUG_PRINTF("  MQTT Enabled: %s\n", config.mqtt_enabled ? "Yes" : "No");
    DEBUG_PRINTF("  Sensorless Mode: %s\n", config.sensorless_mode ? "Yes" : "No");
    DEBUG_PRINTF("  Use Radar: %s\n", config.use_radar ? "Yes" : "No");
}

void saveConfig()
{
    EEPROM.put(0, config);
    EEPROM.commit();
    DEBUG_PRINTLN("Configuration saved to EEPROM");
}

void printFullConfig()
{
    DEBUG_PRINTLN("\n=== FULL CONFIGURATION ===");
    DEBUG_PRINTLN("MQTT Configuration:");
    DEBUG_PRINTF("  Broker: %s:%d\n", config.mqtt_broker, config.mqtt_port);
    DEBUG_PRINTF("  Username: %s\n", config.mqtt_username);
    DEBUG_PRINTF("  Location: %s\n", config.location);
    DEBUG_PRINTF("  Enabled: %s\n", config.mqtt_enabled ? "Yes" : "No");
    DEBUG_PRINTF("  Sensorless Mode: %s\n", config.sensorless_mode ? "Yes" : "No");
    DEBUG_PRINTF("  Use Radar: %s\n", config.use_radar ? "Yes" : "No");
    // ... (add more as needed) ...
    DEBUG_PRINTLN("==========================\n");
}