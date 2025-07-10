#include <Arduino.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include "config.h"
#include "comm/mqtt.h"
#include "web/webserver.h"
#include "model/data_structs.h"
#include "comm/ota.h"
#include "debug/debug_macros.h"
#include "model/config_manager.h"
#include "sensors/pir_sensor.h"
#include "sensors/sensor_manager.h"
#include "comm/wifi_manager.h"
#include "actuators/relay.h"
#include "globals.h"

// Global variables
unsigned long currentTime = 0;
bool ledBlink = false;
unsigned long ledBlinkStart = 0;

void ledInit()
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void setup()
{
    // Initialize serial with a delay to let boot messages clear
    Serial.begin(115200);

    delay(2000); // Wait longer for serial to stabilize

    // Clear any garbage data
    while (Serial.available())
    {
        Serial.read();
    }

    DEBUG_PRINTLN("\n=== ESP8266 Sensor Network ===");
    DEBUG_PRINTLN("Starting setup...");

    // Enable watchdog with longer timeout
    ESP.wdtEnable(WATCHDOG_TIMEOUT);
    ESP.wdtFeed();

    MEMORY_DEBUG_PRINTF("Free heap at start: %d bytes\n", ESP.getFreeHeap());

    // Initialize EEPROM
    DEBUG_PRINTLN("Initializing EEPROM...");
    EEPROM.begin(512);
    ESP.wdtFeed();
    DEBUG_PRINTLN("EEPROM initialized");

    // Load configuration
    DEBUG_PRINTLN("Loading configuration...");
    loadConfig();
    ESP.wdtFeed();

    // Initialize LittleFS
    DEBUG_PRINTLN("Initializing LittleFS...");
    if (!LittleFS.begin())
    {
        DEBUG_PRINTLN("LittleFS mount failed. Formatting...");
        LittleFS.format();
        if (LittleFS.begin())
        {
            DEBUG_PRINTLN("LittleFS successfully formatted and mounted");
        }
        else
        {
            DEBUG_PRINTLN("LittleFS still failed after formatting");
        }
    }

    // Initialize pins
    DEBUG_PRINTLN("Initializing pins...");
    ledInit();
    if(USE_RELAY)
    {
        relayInit();
    }
    
    // relayInit();
    ESP.wdtFeed();
    DEBUG_PRINTLN("Pins initialized");

    // Setup sensors (skip if in sensorless mode)
    if (!config.sensorless_mode)
    {
        DEBUG_PRINTLN("Setting up sensors...");
        setupAllSensors();
        ESP.wdtFeed();
    }
    else
    {
        DEBUG_PRINTLN("Skipping sensor setup (sensorless mode)");
        ESP.wdtFeed();
    }

    // Setup WiFi
    DEBUG_PRINTLN("Setting up WiFi...");
    setupWiFi();
    ESP.wdtFeed();

    // Setup web server
    DEBUG_PRINTLN("Setting up web server...");
    setupWebServer();
    ESP.wdtFeed();

    // Setup MQTT
    DEBUG_PRINTLN("Setting up MQTT...");
    setupMQTT();
    ESP.wdtFeed();

    // Setup OTA
    DEBUG_PRINTLN("Setting up OTA...");
    setupOTA();
    ESP.wdtFeed();

    // PIR interrupt will be managed by updatePirInterrupt() based on sensor availability
    DEBUG_PRINTLN("PIR interrupt management enabled");
    ESP.wdtFeed();

    //Connect to MQTT topic
    if (USE_RELAY)
    {
        relaySetup();
    }

    DEBUG_PRINTLN("Setup complete!");
}

void loop()
{
    currentTime = millis();

    // Read sensors periodically (skip if in sensorless mode)
    if (!config.sensorless_mode && currentTime - lastSensorRead >= SENSOR_READ_INTERVAL)
    {
        readAllSensors();
        printSensorData();
        lastSensorRead = currentTime;
    }

    // Check for OTA updates
    if (currentTime - lastOtaCheck >= OTA_CHECK_INTERVAL)
    {
        DEBUG_PRINTLN("Checking for OTA updates...");
        checkForOTA();
        lastOtaCheck = currentTime;
    }

    // Publish data to MQTT
    if (currentTime - lastMqttPublish >= MQTT_PUBLISH_INTERVAL)
    {
        MQTT_DEBUG_PRINTLN("Publishing data to MQTT...");
        publishData();
        lastMqttPublish = currentTime;
    }

    // Handle MQTT connection
    if (config.mqtt_enabled && !mqttClient.connected())
    {
        connectMQTT();
    }
    mqttClient.loop();

    // Handle PIR cooldown (skip if in sensorless mode)
    if (!config.sensorless_mode && pirTriggered && (currentTime - lastPirTrigger >= PIR_COOLDOWN))
    {
        pirTriggered = false;
        sensorData.motion = false;
    }

    // Handle LED blink for motion detection
    if (ledBlink)
    {
        digitalWrite(LED_PIN, HIGH);
        if (currentTime - ledBlinkStart >= 100) // Blink for 100ms
        {
            digitalWrite(LED_PIN, LOW);
            ledBlink = false;
            DEBUG_PRINTLN("Motion detected!");
        }
    }

    // Handle web server
    static unsigned long lastWebCheck = 0;
    if (currentTime - lastWebCheck >= 1000)
    { // Check every second
        MEMORY_DEBUG_PRINTF("Web server check - Free heap: %d bytes\n", ESP.getFreeHeap());
        lastWebCheck = currentTime;
    }
    server.handleClient();

    // Handle mDNS
    MDNS.update();

    // Feed watchdog and small delay to prevent watchdog issues
    ESP.wdtFeed();

    // Debug: Print memory status every 30 seconds
    static unsigned long lastMemoryCheck = 0;
    if (currentTime - lastMemoryCheck >= 30000) // Every 30 seconds
    {
        MEMORY_DEBUG_PRINTF("Debug - Free heap: %d bytes, Uptime: %lu ms\n", ESP.getFreeHeap(), currentTime);
        lastMemoryCheck = currentTime;
    }

    delay(100);
}
