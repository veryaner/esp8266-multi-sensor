#include <Arduino.h>
#include "config.h"
#include "actuators/relay.h"
#include "comm/mqtt.h"

bool relayState = false;

void relayInit()
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
}
void relaySetup()
{
    subscribe(MQTT_TOPIC_RELAY);
}

bool getRelayState()
{
    return relayState;
}

int getRelayPin()
{
    return RELAY_PIN;
}

void setRelayState(bool state)
{
    relayState = state;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
}