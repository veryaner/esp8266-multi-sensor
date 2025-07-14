#include <Arduino.h>
#include <PubSubClient.h>
#include "config.h"
#include "comm/mqtt.h"
#include "model/data_structs.h"
#include "debug/debug_macros.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "sensors/sensor_manager.h"
#include "actuators/relay.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool mqttConnected = false;
unsigned long lastMqttPublish = 0;

void setupMQTT()
{
    if (!config.mqtt_enabled)
    {
        MQTT_DEBUG_PRINTLN("MQTT disabled in configuration");
        return;
    }

    MQTT_DEBUG_PRINTLN("Setting up MQTT...");

    // Set MQTT server and callback
    mqttClient.setServer(config.mqtt_broker, config.mqtt_port);
    mqttClient.setCallback(mqttCallback);

    // Set buffer size for larger messages
    mqttClient.setBufferSize(512);

    MQTT_DEBUG_PRINT("MQTT broker: ");
    MQTT_DEBUG_PRINT(config.mqtt_broker);
    MQTT_DEBUG_PRINT(":");
    MQTT_DEBUG_PRINTLN(config.mqtt_port);
    MQTT_DEBUG_PRINT("Location: ");
    MQTT_DEBUG_PRINTLN(config.location);

    // Try to connect
    connectMQTT();
}

bool connectMQTT()
{
    if (!config.mqtt_enabled)
        return false;

    MQTT_DEBUG_PRINT("Connecting to MQTT broker...");

    // Create a random client ID
    String clientId = String(MQTT_CLIENT_ID) + "-" + String(random(0xffff), HEX);

    // Attempt to connect
    if (mqttClient.connect(clientId.c_str(), config.mqtt_username, config.mqtt_password))
    {
        MQTT_DEBUG_PRINTLN(" connected!");
        mqttConnected = true;

        // Subscribe to command topics
        if (USE_RELAY)
        {
            mqttClient.subscribe(getTopicWithLocation(MQTT_TOPIC_RELAY_COMMAND).c_str());
        }

        // Publish initial status
        mqttClient.publish(getTopicWithLocation(MQTT_TOPIC_STATUS).c_str(), "online");

        return true;
    }
    else
    {
        MQTT_DEBUG_PRINTF(" failed, rc=%d\n", mqttClient.state());
        mqttConnected = false;
        return false;
    }
}

void subscribe(char *topic)
{
    mqttClient.subscribe(getTopicWithLocation(topic).c_str());
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    MQTT_DEBUG_PRINTF("Message arrived on topic: %s\n", topic);

    // Convert payload to string
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    MQTT_DEBUG_PRINTF("Message: %s\n", message);

    // Check if this is a relay command
    String topicStr = String(topic);
    String relayCommandTopic = getTopicWithLocation(MQTT_TOPIC_RELAY_COMMAND);

    if (topicStr == relayCommandTopic)
    {
        handleRelayCommand(message);
        return;
    }

    // Handle other topics if needed
    // Example: if (String(topic) == "sensors/command") { ... }
}

void handleRelayCommand(const char *message)
{
    MQTT_DEBUG_PRINTF("Processing relay command: %s\n", message);

    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
        MQTT_DEBUG_PRINTF("Failed to parse relay command JSON: %s\n", error.c_str());
        return;
    }

    bool newState = false;
    bool shouldToggle = false;

    if (doc.containsKey("command"))
    {
        String command = doc["command"];
        if (command == "toggle")
        {
            shouldToggle = true;
        }
        else if (command == "set" && doc.containsKey("state"))
        {
            newState = doc["state"];
        }
    }
    else if (doc.containsKey("state"))
    {
        // Direct state setting for backward compatibility
        newState = doc["state"];
    }
    else
    {
        MQTT_DEBUG_PRINT("Invalid relay command format\n");
        return;
    }

    if (shouldToggle)
    {
        newState = !getRelayState();
    }

    // Set the relay state
    setRelayState(newState);

    // Publish the new state back to status topic (not command topic)
    publishRelayState();

    MQTT_DEBUG_PRINTF("Relay set to: %s\n", newState ? "ON" : "OFF");
}

void publishRelayState()
{
    if (!mqttClient.connected())
    {
        MQTT_DEBUG_PRINTLN("MQTT not connected, skipping publish");
        return;
    }

    DynamicJsonDocument doc(256);
    doc["state"] = getRelayState();
    doc["pin"] = getRelayPin();
    doc["timestamp"] = millis();
    doc["location"] = config.location;

    String message;
    serializeJson(doc, message);

    String topic = getTopicWithLocation(MQTT_TOPIC_RELAY_STATUS);
    mqttClient.publish(topic.c_str(), message.c_str());

    MQTT_DEBUG_PRINTF("Published relay state: %s to topic: %s\n", message.c_str(), topic.c_str());
}

void publishData()
{
    if (!mqttClient.connected())
    {
        MQTT_DEBUG_PRINTLN("MQTT not connected, skipping publish");
        return;
    }

    // Publish individual sensor data
    if (config.use_dht && sensorData.dht_available)
    {
        String topic = getTopicWithLocation(MQTT_TOPIC_TEMPERATURE);
        mqttClient.publish(topic.c_str(), String(sensorData.temperature).c_str());

        topic = getTopicWithLocation(MQTT_TOPIC_HUMIDITY);
        mqttClient.publish(topic.c_str(), String(sensorData.humidity).c_str());
    }

    if (config.use_tsl2561 && sensorData.tsl_available)
    {
        String topic = getTopicWithLocation(MQTT_TOPIC_LUMINESCENCE);
        mqttClient.publish(topic.c_str(), String(sensorData.lux).c_str());
    }

    if (config.use_pir && sensorData.pir_available)
    {
        String topic = getTopicWithLocation(MQTT_TOPIC_MOTION);
        mqttClient.publish(topic.c_str(), sensorData.motion ? "1" : "0");
    }

    if (config.use_ld2410 && sensorData.radar_available)
    {
        String topic = getTopicWithLocation(MQTT_TOPIC_RADAR_PRESENCE);
        mqttClient.publish(topic.c_str(), sensorData.radar_presence ? "1" : "0");
    }

    if (config.use_relay)
    {
        publishRelayState();
    }

    // Publish all sensor data as JSON
    DynamicJsonDocument doc(512);
    doc["temperature"] = sensorData.temperature;
    doc["humidity"] = sensorData.humidity;
    doc["motion"] = sensorData.motion;
    doc["luminescence"] = sensorData.lux;
    doc["radar_presence"] = sensorData.radar_presence;
    doc["relay_state"] = getRelayState();
    doc["relay_pin"] = getRelayPin();
    doc["timestamp"] = sensorData.timestamp;
    doc["location"] = config.location;
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI();

    String message;
    serializeJson(doc, message);

    String topic = getTopicWithLocation(MQTT_TOPIC_ALL);
    mqttClient.publish(topic.c_str(), message.c_str());

    MQTT_DEBUG_PRINTF("Published all sensor data: %s\n", message.c_str());
}

String getTopicWithLocation(const char *topic)
{
    String fullTopic = String(config.location) + "/" + String(topic);
    return fullTopic;
}
