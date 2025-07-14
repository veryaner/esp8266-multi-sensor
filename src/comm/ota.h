#pragma once
#include <ESP8266WebServer.h>
void setupOTA();
void checkForOTA();
void setupOTA_HTTP(ESP8266WebServer &server);