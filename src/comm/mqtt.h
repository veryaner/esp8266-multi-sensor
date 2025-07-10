#pragma once
#include <PubSubClient.h>
extern PubSubClient mqttClient;
extern unsigned long lastMqttPublish;
extern bool mqttConnected;
void setupMQTT();
bool connectMQTT();
void publishSensorData();
void mqttCallback(char *topic, byte *payload, unsigned int length);
String getTopicWithLocation(const char *topic);
void handleRelayCommand(const char *message);
void publishRelayState();