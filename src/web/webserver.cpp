#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "config.h"
#include "comm/wifi_manager.h"
#include "web/webserver.h"
#include "comm/mqtt.h"
#include "model/data_structs.h"
#include "debug/debug_macros.h"
#include "sensors/sensor_manager.h"
#include "model/config_manager.h"
#include "actuators/relay.h"

ESP8266WebServer server;

void setupWebServer()
{
    WEB_DEBUG_PRINTLN("Setting up web server...");
    MEMORY_DEBUG_PRINTF("Free heap before web server setup: %d bytes\n", ESP.getFreeHeap());

    // Check if LittleFS is available
    if (LittleFS.begin())
    {
        WEB_DEBUG_PRINTLN("LittleFS available - setting up static file serving");
        // Serve static files from LittleFS, but don't override API endpoints
        server.serveStatic("/static", LittleFS, "/", "max-age=86400");
        server.onNotFound([]()
                          {
            WEB_DEBUG_PRINTF("404 Not Found: %s\n", server.uri().c_str());
            // Only serve static files for /static/ paths
            if (server.uri().startsWith("/static/") && LittleFS.exists(server.uri().substring(8))) {
                server.serveStatic(server.uri().c_str(), LittleFS, server.uri().substring(8).c_str());
            } else {
                server.send(404, "text/plain", "Not Found");
            } });
    }
    else
    {
        WEB_DEBUG_PRINTLN("LittleFS not available - setting up basic API endpoints only");
    }

    // Set up a basic response for the root path (works with or without LittleFS)
    server.on("/", HTTP_GET, []()
              {
        if (LittleFS.exists("/index.html")) {
            File file = LittleFS.open("/index.html", "r");
            server.streamFile(file, "text/html");
            file.close();
        } else {
            server.send(404, "text/plain", "index.html not found");
        } });

    // API endpoints
    server.on("/api/sensors", HTTP_GET, []()
              {
        WEB_DEBUG_PRINTLN("=== API /api/sensors requested ===");
        MEMORY_DEBUG_PRINTF("Free heap before API call: %d bytes\n", ESP.getFreeHeap());
        WEB_DEBUG_PRINTLN("Starting sensor data collection...");
        
        // Add CORS headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        readAllSensors(); // Get fresh data
        
        // Create a simpler JSON structure first
        DynamicJsonDocument doc(512);
        
        // Basic sensor data
        doc["temperature"] = sensorData.temperature;
        doc["humidity"] = sensorData.humidity;
        doc["motion"] = sensorData.presence;
        doc["luminescence"] = sensorData.lux;
        doc["timestamp"] = sensorData.timestamp;
        doc["uptime"] = millis();
        doc["wifi_rssi"] = WiFi.RSSI();
        doc["free_heap"] = ESP.getFreeHeap();
        doc["sensorless_mode"] = config.sensorless_mode;
        
        String actualMotionSource = "none";
        if (config.use_ld2410 && sensorData.radar_available) {
            actualMotionSource = "radar";
        }
        else if (config.use_pir && sensorData.pir_available)
        {
            actualMotionSource = "pir";
        }
        doc["motion_source"] = actualMotionSource;
        doc["motion"] = (actualMotionSource == "radar") ? sensorData.radar_presence : sensorData.presence;
        
        // Simple sensor status (avoid nested objects for now)
        doc["dht11_available"] = sensorData.dht_available;
        doc["dht11_errors"] = sensorData.dht_error_count;
        doc["tsl2561_available"] = sensorData.tsl_available;
        doc["tsl2561_errors"] = sensorData.tsl_error_count;
        doc["pir_available"] = sensorData.pir_available;
        doc["pir_errors"] = sensorData.pir_error_count;
        doc["radar_presence"] = sensorData.radar_presence;
        doc["relay_state"] = getRelayState();
        doc["relay_pin"] = getRelayPin();

        String response;
        serializeJson(doc, response);
        
        WEB_DEBUG_PRINTF("API response length: %d bytes\n", response.length());
        WEB_DEBUG_PRINTF("API response: %s\n", response.c_str());
        MEMORY_DEBUG_PRINTF("Free heap after API call: %d bytes\n", ESP.getFreeHeap());
        
        server.send(200, "application/json", response); });

    server.on("/api/restart", HTTP_POST, []()
              {
        server.send(200, "text/plain", "Restarting...");
        delay(1000);
        ESP.restart(); });

    server.on("/api/wifi/reset", HTTP_POST, []()
              {
        server.send(200, "text/plain", "Resetting WiFi settings...");
        wifiManager.resetSettings();
        ESP.restart(); });

    server.on("/api/factory-reset", HTTP_POST, []()
              {
        server.send(200, "text/plain", "Performing factory reset...");
        // Clear EEPROM
        for (int i = 0; i < 512; i++) {
            EEPROM.write(i, 0);
        }
        EEPROM.commit();
        wifiManager.resetSettings();
        ESP.restart(); });

    server.on("/api/config", HTTP_GET, []()
              {
        // Add CORS headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        DynamicJsonDocument doc(512);
        doc["mqtt_broker"] = config.mqtt_broker;
        doc["mqtt_port"] = config.mqtt_port;
        doc["mqtt_username"] = config.mqtt_username;
        doc["location"] = config.location;
        doc["mqtt_enabled"] = config.mqtt_enabled;
        doc["sensorless_mode"] = config.sensorless_mode;

        
        // Add sensor enable flags
        doc["use_dht"] = config.use_dht;
        doc["use_tsl2561"] = config.use_tsl2561;
        doc["use_pir"] = config.use_pir;
        doc["use_ld2410"] = config.use_ld2410;
        doc["use_relay"] = config.use_relay;
        
        String response;
        serializeJson(doc, response);
        
        server.send(200, "application/json", response); });

    server.on("/api/config", HTTP_POST, []()
              {
        // Add CORS headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        String body = server.arg("plain");
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, body);
        
        if (error) {
            server.send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
            return;
        }
        
        bool configChanged = false;
        
        // Update MQTT settings
        if (doc.containsKey("mqtt_broker")) {
            strlcpy(config.mqtt_broker, doc["mqtt_broker"], sizeof(config.mqtt_broker));
            configChanged = true;
        }
        if (doc.containsKey("mqtt_port")) {
            config.mqtt_port = doc["mqtt_port"];
            configChanged = true;
        }
        if (doc.containsKey("mqtt_username")) {
            strlcpy(config.mqtt_username, doc["mqtt_username"], sizeof(config.mqtt_username));
            configChanged = true;
        }
        if (doc.containsKey("mqtt_password")) {
            strlcpy(config.mqtt_password, doc["mqtt_password"], sizeof(config.mqtt_password));
            configChanged = true;
        }
        if (doc.containsKey("mqtt_enabled")) {
            config.mqtt_enabled = doc["mqtt_enabled"];
            configChanged = true;
        }
        
        // Update device settings
        if (doc.containsKey("location")) {
            strlcpy(config.location, doc["location"], sizeof(config.location));
            configChanged = true;
        }
        if (doc.containsKey("sensorless_mode")) {
            config.sensorless_mode = doc["sensorless_mode"];
            configChanged = true;
        }
        
        // Handle sensor enable flags
        if (doc.containsKey("use_dht")) {
            config.use_dht = doc["use_dht"];
            configChanged = true;
        }
        
        if (doc.containsKey("use_tsl2561")) {
            config.use_tsl2561 = doc["use_tsl2561"];
            configChanged = true;
        }
        
        if (doc.containsKey("use_pir")) {
            config.use_pir = doc["use_pir"];
            configChanged = true;
        }
        
        if (doc.containsKey("use_ld2410")) {
            config.use_ld2410 = doc["use_ld2410"];
            configChanged = true;
        }
        
        if (doc.containsKey("use_relay")) {
            config.use_relay = doc["use_relay"];
            configChanged = true;
        }
        
        if (configChanged) {
            saveConfig();
            DynamicJsonDocument responseDoc(128);
            responseDoc["message"] = "Configuration updated successfully";
            responseDoc["timestamp"] = millis();
            
            String response;
            serializeJson(responseDoc, response);
            server.send(200, "application/json", response);
        } else {
            server.send(400, "application/json", "{\"error\": \"No valid configuration parameters provided\"}");
        } });

    // Export full configuration including pin assignments
    server.on("/api/config/full", HTTP_GET, []()
              {
        DynamicJsonDocument doc(2048);
        
        // Current configuration
        doc["mqtt_broker"] = config.mqtt_broker;
        doc["mqtt_port"] = config.mqtt_port;
        doc["mqtt_username"] = config.mqtt_username;
        doc["location"] = config.location;
        doc["mqtt_enabled"] = config.mqtt_enabled;
        doc["sensorless_mode"] = config.sensorless_mode;
        
        // Pin assignments
        doc["pins"]["dht11"] = DHT_PIN;
        doc["pins"]["pir"] = PIR_PIN;
        doc["pins"]["led"] = LED_PIN;
        doc["pins"]["sda"] = SDA_PIN;
        doc["pins"]["scl"] = SCL_PIN;
        
        // System information
        doc["system"]["device_hostname"] = deviceHostname; // Use MDNS.hostname() for the actual hostname
        doc["system"]["wifi_ssid"] = WIFI_SSID;
        doc["system"]["wifi_password"] = WIFI_PASSWORD;
        doc["system"]["ap_ip"] = AP_IP;
        
        // Timing configuration
        doc["timing"]["sensor_read_interval"] = SENSOR_READ_INTERVAL;
        doc["timing"]["mqtt_publish_interval"] = MQTT_PUBLISH_INTERVAL;
        doc["timing"]["pir_cooldown"] = PIR_COOLDOWN;
        doc["timing"]["ota_check_interval"] = OTA_CHECK_INTERVAL;
        
        // MQTT topics
        doc["mqtt_topics"]["temperature"] = MQTT_TOPIC_TEMPERATURE;
        doc["mqtt_topics"]["humidity"] = MQTT_TOPIC_HUMIDITY;
        doc["mqtt_topics"]["motion"] = MQTT_TOPIC_MOTION;
        doc["mqtt_topics"]["luminescence"] = MQTT_TOPIC_LUMINESCENCE;
        doc["mqtt_topics"]["status"] = MQTT_TOPIC_STATUS;
        doc["mqtt_topics"]["all"] = MQTT_TOPIC_ALL;
        
        String response;
        serializeJson(doc, response);
        
        server.send(200, "application/json", response); });

    // Export configuration to file
    server.on("/api/config/export", HTTP_GET, []()
              {
        DynamicJsonDocument doc(2048);
        
        // Current configuration
        doc["mqtt_broker"] = config.mqtt_broker;
        doc["mqtt_port"] = config.mqtt_port;
        doc["mqtt_username"] = config.mqtt_username;
        doc["mqtt_password"] = config.mqtt_password;
        doc["location"] = config.location;
        doc["mqtt_enabled"] = config.mqtt_enabled;
        doc["sensorless_mode"] = config.sensorless_mode;

        
        // Pin assignments
        doc["pins"]["dht11"] = DHT_PIN;
        doc["pins"]["pir"] = PIR_PIN;
        doc["pins"]["led"] = LED_PIN;
        doc["pins"]["sda"] = SDA_PIN;
        doc["pins"]["scl"] = SCL_PIN;
        
        // System information
        doc["system"]["device_hostname"] = deviceHostname; // Use MDNS.hostname() for the actual hostname
        doc["system"]["wifi_ssid"] = WIFI_SSID;
        doc["system"]["wifi_password"] = WIFI_PASSWORD;
        doc["system"]["ap_ip"] = AP_IP;
        
        // Timing configuration
        doc["timing"]["sensor_read_interval"] = SENSOR_READ_INTERVAL;
        doc["timing"]["mqtt_publish_interval"] = MQTT_PUBLISH_INTERVAL;
        doc["timing"]["pir_cooldown"] = PIR_COOLDOWN;
        doc["timing"]["ota_check_interval"] = OTA_CHECK_INTERVAL;
        
        // MQTT topics
        doc["mqtt_topics"]["temperature"] = MQTT_TOPIC_TEMPERATURE;
        doc["mqtt_topics"]["humidity"] = MQTT_TOPIC_HUMIDITY;
        doc["mqtt_topics"]["motion"] = MQTT_TOPIC_MOTION;
        doc["mqtt_topics"]["luminescence"] = MQTT_TOPIC_LUMINESCENCE;
        doc["mqtt_topics"]["status"] = MQTT_TOPIC_STATUS;
        doc["mqtt_topics"]["all"] = MQTT_TOPIC_ALL;
        
        // Export timestamp
        doc["export_timestamp"] = millis();
        doc["export_uptime"] = millis();
        
        String response;
        serializeJsonPretty(doc, response);
        
        server.sendHeader("Content-Disposition", "attachment; filename=esp8266_config.json");
        server.send(200, "application/json", response); });

    // Print full configuration to serial monitor
    server.on("/api/config/print", HTTP_POST, []()
              {
        DEBUG_PRINTLN("Full configuration requested via API...");
        printFullConfig();
        
        DynamicJsonDocument responseDoc(128);
        responseDoc["message"] = "Configuration printed to serial monitor";
        responseDoc["timestamp"] = millis();
        
        String response;
        serializeJson(responseDoc, response);
        server.send(200, "application/json", response); });

    server.on("/api/sensorless", HTTP_POST, []()
              {
        String body = server.arg("plain");
        DynamicJsonDocument doc(256);
        deserializeJson(doc, body);
        
        if (doc.containsKey("enabled"))
        {
            config.sensorless_mode = doc["enabled"];
            saveConfig();
            
            DynamicJsonDocument responseDoc(128);
            responseDoc["sensorless_mode"] = config.sensorless_mode;
            responseDoc["message"] = config.sensorless_mode ? "Sensorless mode enabled" : "Sensorless mode disabled";
            
            String response;
            serializeJson(responseDoc, response);
            server.send(200, "application/json", response);
        }
        else
        {
            server.send(400, "application/json", "{\"error\": \"Missing 'enabled' parameter\"}");
        } });

    // Simple test endpoint
    server.on("/test", HTTP_GET, []()
              {
        WEB_DEBUG_PRINTLN("Test endpoint requested");
        MEMORY_DEBUG_PRINTF("Free heap: %d bytes\n", ESP.getFreeHeap());
        server.send(200, "text/plain", "ESP8266 is working! Free heap: " + String(ESP.getFreeHeap()) + " bytes"); });

    // Very simple API test endpoint
    server.on("/api/test", HTTP_GET, []()
              {
        WEB_DEBUG_PRINTLN("API test endpoint requested");
        server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"API is working\"}"); });

    // Ultra simple test endpoint
    server.on("/simple", HTTP_GET, []()
              {
        WEB_DEBUG_PRINTLN("Simple endpoint requested");
        server.send(200, "text/plain", "Simple endpoint works!"); });

    // Health check endpoint
    server.on("/health", HTTP_GET, []()
              {
        WEB_DEBUG_PRINTLN("Health check requested");
        DynamicJsonDocument doc(256);
        doc["status"] = "ok";
        doc["uptime"] = millis();
        doc["free_heap"] = ESP.getFreeHeap();
        doc["wifi_rssi"] = WiFi.RSSI();
        doc["ip"] = WiFi.localIP().toString();
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response); });

    // Debug endpoint for sensor data
    server.on("/debug/sensors", HTTP_GET, []()
              {
        WEB_DEBUG_PRINTLN("Debug sensors endpoint requested");
        
        // Add CORS headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        // Create a simple response first
        DynamicJsonDocument doc(256);
        doc["test"] = "sensor_data";
        doc["free_heap"] = ESP.getFreeHeap();
        doc["uptime"] = millis();
        
        // Add sensor data step by step
        doc["temperature"] = sensorData.temperature;
        doc["humidity"] = sensorData.humidity;
        doc["motion"] = sensorData.presence;
        doc["luminescence"] = sensorData.lux;
        doc["timestamp"] = sensorData.timestamp;
        doc["radar_presence"] = sensorData.radar_presence;
        
        // Add sensor status
        doc["dht_available"] = sensorData.dht_available;
        doc["tsl_available"] = sensorData.tsl_available;
        doc["pir_available"] = sensorData.pir_available;
        doc["radar_available"] = sensorData.radar_available;
        
        String response;
        serializeJson(doc, response);
        
        WEB_DEBUG_PRINTF("Debug response length: %d bytes\n", response.length());
        WEB_DEBUG_PRINTF("Debug response: %s\n", response.c_str());
        
        server.send(200, "application/json", response); });

    // Reset sensor status endpoint
    server.on("/api/sensors/reset", HTTP_POST, []()
              {
        WEB_DEBUG_PRINTLN("Resetting sensor status...");
        
        // Reset all error counters
        sensorData.dht_error_count = 0;
        sensorData.tsl_error_count = 0;
        sensorData.pir_error_count = 0;
        
        // Mark all sensors as available for retry
        sensorData.dht_available = true;
        sensorData.tsl_available = true;
        sensorData.pir_available = true;
        sensorData.radar_available = true;
        
        config.sensorless_mode = false;
        saveConfig();
        
        DynamicJsonDocument responseDoc(256);
        responseDoc["message"] = "Sensor status reset - all sensors marked as available";
        responseDoc["sensorless_mode"] = false;
        responseDoc["sensors"]["dht11"]["available"] = true;
        responseDoc["sensors"]["dht11"]["error_count"] = 0;
        responseDoc["sensors"]["tsl2561"]["available"] = true;
        responseDoc["sensors"]["tsl2561"]["error_count"] = 0;
        responseDoc["sensors"]["pir"]["available"] = true;
        responseDoc["sensors"]["pir"]["error_count"] = 0;
        responseDoc["sensors"]["radar"]["available"] = true;
        responseDoc["sensors"]["radar"]["error_count"] = 0;
        
        String response;
        serializeJson(responseDoc, response);
        server.send(200, "application/json", response); });

    // Individual sensor reset endpoints
    server.on("/api/sensors/dht11/reset", HTTP_POST, []()
              {
        WEB_DEBUG_PRINTLN("Resetting DHT11 sensor status...");
        sensorData.dht_error_count = 0;
        sensorData.dht_available = true;
        
        DynamicJsonDocument responseDoc(128);
        responseDoc["message"] = "DHT11 sensor reset";
        responseDoc["sensors"]["dht11"]["available"] = true;
        responseDoc["sensors"]["dht11"]["error_count"] = 0;
        
        String response;
        serializeJson(responseDoc, response);
        server.send(200, "application/json", response); });

    server.on("/api/sensors/tsl2561/reset", HTTP_POST, []()
              {
        WEB_DEBUG_PRINTLN("Resetting TSL2561 sensor status...");
        sensorData.tsl_error_count = 0;
        sensorData.tsl_available = true;
        
        DynamicJsonDocument responseDoc(128);
        responseDoc["message"] = "TSL2561 sensor reset";
        responseDoc["sensors"]["tsl2561"]["available"] = true;
        responseDoc["sensors"]["tsl2561"]["error_count"] = 0;
        
        String response;
        serializeJson(responseDoc, response);
        server.send(200, "application/json", response); });

    server.on("/api/sensors/pir/reset", HTTP_POST, []()
              {
        WEB_DEBUG_PRINTLN("Resetting PIR sensor status...");
        sensorData.pir_error_count = 0;
        sensorData.pir_available = true;
        
        DynamicJsonDocument responseDoc(128);
        responseDoc["message"] = "PIR sensor reset";
        responseDoc["sensors"]["pir"]["available"] = true;
        responseDoc["sensors"]["pir"]["error_count"] = 0;
        
        String response;
        serializeJson(responseDoc, response);
        server.send(200, "application/json", response); });

    // Handle CORS preflight requests
    server.on("/api/sensors", HTTP_OPTIONS, []()
              {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(200, "text/plain", ""); });

    // Debug mode toggle endpoint
    server.on("/api/debug", HTTP_POST, []()
              {
        String body = server.arg("plain");
        DynamicJsonDocument doc(256);
        deserializeJson(doc, body);
        
        if (doc.containsKey("enabled"))
        {
            bool debugEnabled = doc["enabled"];
            // Note: This would require recompiling to take effect
            // For now, just return the current status
            
            DynamicJsonDocument responseDoc(128);
            responseDoc["debug_mode"] = debugEnabled;
            responseDoc["message"] = debugEnabled ? "Debug mode enabled (requires restart)" : "Debug mode disabled (requires restart)";
            responseDoc["note"] = "Debug mode changes require firmware recompilation";
            
            String response;
            serializeJson(responseDoc, response);
            server.send(200, "application/json", response);
        }
        else
        {
            server.send(400, "application/json", "{\"error\": \"Missing 'enabled' parameter\"}");
        } });

    // Sensor configuration endpoint for frontend
    server.on("/api/sensor-config", HTTP_GET, []()
              {
        // Add CORS headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        DynamicJsonDocument doc(256);
        doc["use_dht"] = config.use_dht;
        doc["use_tsl2561"] = config.use_tsl2561;
        doc["use_pir"] = config.use_pir;
        doc["use_ld2410"] = config.use_ld2410;
        doc["use_relay"] = config.use_relay;
        
        String response;
        serializeJson(doc, response);
        
        server.send(200, "application/json", response); });

    // Relay API endpoints
    server.on("/api/relay", HTTP_GET, []()
              {
        // Add CORS headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        DynamicJsonDocument doc(128);
        doc["state"] = getRelayState();
        doc["pin"] = getRelayPin();
        
        String response;
        serializeJson(doc, response);
        
        server.send(200, "application/json", response); });

    server.on("/api/relay", HTTP_POST, []()
              {
        // Add CORS headers
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        String body = server.arg("plain");
        DynamicJsonDocument doc(128);
        deserializeJson(doc, body);
        
        bool newState = false;
        
        if (doc.containsKey("state")) {
            newState = doc["state"];
            setRelayState(newState);
        } else {
            // Toggle if no state specified
            newState = !getRelayState();
            setRelayState(newState);
        }
        
        DynamicJsonDocument responseDoc(128);
        responseDoc["state"] = newState;
        responseDoc["pin"] = getRelayPin();
        responseDoc["message"] = newState ? "Relay turned ON" : "Relay turned OFF";
        
        String response;
        serializeJson(responseDoc, response);
        
        server.send(200, "application/json", response); });

    server.begin();
    WEB_DEBUG_PRINTLN("Web server started");
    MEMORY_DEBUG_PRINTF("Free heap after web server setup: %d bytes\n", ESP.getFreeHeap());

    // Debug: List all registered endpoints
    WEB_DEBUG_PRINTLN("Registered endpoints:");
    WEB_DEBUG_PRINTLN("- GET /");
    WEB_DEBUG_PRINTLN("- GET /api/sensors");
    WEB_DEBUG_PRINTLN("- GET /api/config");
    WEB_DEBUG_PRINTLN("- GET /health");
    WEB_DEBUG_PRINTLN("- GET /test");
    WEB_DEBUG_PRINTLN("- GET /simple");
    WEB_DEBUG_PRINTLN("- POST /api/restart");
    WEB_DEBUG_PRINTLN("- POST /api/wifi/reset");
    WEB_DEBUG_PRINTLN("- POST /api/sensors/reset");
}