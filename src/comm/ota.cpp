#include "comm/ota.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
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

void setupArduinoOTA()
{
    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setHostname(DEVICE_HOSTNAME);

    ArduinoOTA.onStart([]()
                       {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_FS
            type = "filesystem";
        Serial.println("[OTA] Start updating " + type); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("[OTA] End"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
        Serial.printf("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
    ArduinoOTA.begin();
    Serial.println("[OTA] ArduinoOTA ready");
}

void handleArduinoOTA()
{
    ArduinoOTA.handle();
}