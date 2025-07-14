#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

extern String deviceHostname;
extern WiFiManager wifiManager;
extern NTPClient timeClient;

void setupWiFi(bool forceConfigPortal);
String sanitizeLocation(const String &location);

#endif // WIFI_MANAGER_H 