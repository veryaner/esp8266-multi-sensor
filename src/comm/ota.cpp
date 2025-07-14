#include "comm/ota.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "config.h"

ESP8266HTTPUpdateServer httpUpdater;

void setupOTA_HTTP(ESP8266WebServer &server)
{
    // Set up the /update endpoint for OTA firmware upload with authentication
    httpUpdater.setup(&server, "/update", OTA_PASSWORD);
}

void setupOTA()
{
    // Deprecated: Use setupOTA_HTTP with the web server instance
}

void checkForOTA()
{
    // Not needed for HTTP OTA; handled by web server
}