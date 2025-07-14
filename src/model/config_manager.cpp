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
        // config.use_radar = DEFAULT_USE_RADAR;

        // Initialize sensor enable flags from compile-time defines
        config.use_dht = USE_DHT;
        config.use_tsl2561 = USE_TSL2561;
        config.use_pir = USE_PIR;
        config.use_ld2410 = USE_LD2410;
        config.use_relay = USE_RELAY;

        saveConfig();
    }
    config.sensorless_mode = DEFAULT_SENSORLESS_MODE;
    DEBUG_PRINTLN("Configuration loaded:");
    DEBUG_PRINTF("  MQTT Broker: %s:%d\n", config.mqtt_broker, config.mqtt_port);
    DEBUG_PRINTF("  Location: %s\n", config.location);
    DEBUG_PRINTF("  MQTT Enabled: %s\n", config.mqtt_enabled ? "Yes" : "No");
    DEBUG_PRINTF("  Sensorless Mode: %s\n", config.sensorless_mode ? "Yes" : "No");
    DEBUG_PRINTF("  Sensors - DHT: %s, TSL: %s, PIR: %s, LD2410: %s, Relay: %s\n",
                 config.use_dht ? "Yes" : "No",
                 config.use_tsl2561 ? "Yes" : "No",
                 config.use_pir ? "Yes" : "No",
                 config.use_ld2410 ? "Yes" : "No",
                 config.use_relay ? "Yes" : "No");
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
    DEBUG_PRINTF("  Sensors - DHT: %s, TSL: %s, PIR: %s, LD2410: %s, Relay: %s\n",
                 config.use_dht ? "Yes" : "No",
                 config.use_tsl2561 ? "Yes" : "No",
                 config.use_pir ? "Yes" : "No",
                 config.use_ld2410 ? "Yes" : "No",
                 config.use_relay ? "Yes" : "No");
    // ... (add more as needed) ...
    DEBUG_PRINTLN("==========================\n");
}