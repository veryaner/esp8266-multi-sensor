#include "config.h"
#include "comm/wifi_manager.h"
#include "debug/debug_macros.h"
#include "model/config_manager.h"

String deviceHostname = "";
WiFiManager wifiManager;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setupWiFi(bool forceConfigPortal)
{
    DEBUG_PRINTLN("Setting up WiFi...");

    wifiManager.setAPCallback([](WiFiManager *myWiFiManager)
                              {
        DEBUG_PRINTLN("Entered config mode");
        DEBUG_PRINTLN(WiFi.softAPIP());
        DEBUG_PRINTLN(myWiFiManager->getConfigPortalSSID()); });

    WiFiManagerParameter custom_mqtt_broker("mqtt_broker", "MQTT Broker IP", config.mqtt_broker, 32);
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", String(config.mqtt_port).c_str(), 6);
    WiFiManagerParameter custom_mqtt_username("mqtt_username", "MQTT Username (optional)", config.mqtt_username, 32);
    WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT Password (optional)", config.mqtt_password, 32);
    WiFiManagerParameter custom_location("location", "Device Location (e.g., living_room, bedroom)", config.location, 32);
    WiFiManagerParameter custom_mqtt_enabled("mqtt_enabled", "Enable MQTT (1=yes, 0=no)", config.mqtt_enabled ? "1" : "0", 2);
    WiFiManagerParameter custom_sensorless_mode("sensorless_mode", "Sensorless Mode (1=yes, 0=no)", config.sensorless_mode ? "1" : "0", 2);

    wifiManager.addParameter(&custom_mqtt_broker);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_username);
    wifiManager.addParameter(&custom_mqtt_password);
    wifiManager.addParameter(&custom_location);
    wifiManager.addParameter(&custom_mqtt_enabled);
    wifiManager.addParameter(&custom_sensorless_mode);

    wifiManager.setConfigPortalTimeout(180); // 3 minutes timeout

    bool connected = false;
    if (forceConfigPortal)
    {
        DEBUG_PRINTLN("Forcing WiFiManager config portal...");
        connected = wifiManager.startConfigPortal(WIFI_SSID, WIFI_PASSWORD);
    }
    else
    {
        connected = wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD);
    }
    if (!connected)
    {
        DEBUG_PRINTLN("Failed to connect and hit timeout");
        ESP.restart();
    }

    strcpy(config.mqtt_broker, custom_mqtt_broker.getValue());
    config.mqtt_port = atoi(custom_mqtt_port.getValue());
    strcpy(config.mqtt_username, custom_mqtt_username.getValue());
    strcpy(config.mqtt_password, custom_mqtt_password.getValue());
    strcpy(config.location, custom_location.getValue());
    config.mqtt_enabled = (strcmp(custom_mqtt_enabled.getValue(), "1") == 0);
    config.sensorless_mode = (strcmp(custom_sensorless_mode.getValue(), "1") == 0);

    saveConfig();

    DEBUG_PRINTLN("WiFi connected!");
    DEBUG_PRINT("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());

    if (config.mqtt_enabled)
    {
        DEBUG_PRINTLN("MQTT Configuration:");
        DEBUG_PRINTF("  Broker: %s:%d\n", config.mqtt_broker, config.mqtt_port);
        DEBUG_PRINTF("  Location: %s\n", config.location);
        DEBUG_PRINTF("  Username: %s\n", config.mqtt_username);
    }

    if (config.sensorless_mode)
    {
        DEBUG_PRINTLN("Sensorless Mode: ENABLED - No sensors will be used");
    }
    else
    {
        DEBUG_PRINTLN("Sensorless Mode: DISABLED - All sensors will be used");
    }

    // Initialize NTP
    timeClient.begin();
    timeClient.setTimeOffset(0); // Adjust for your timezone

    // Setup mDNS

    String sanitizedLocation = sanitizeLocation(String(config.location));
    deviceHostname = "esp8266-" + sanitizedLocation;
    Serial.print("Device Hostname: ");
    Serial.println(deviceHostname);
    if (MDNS.begin(deviceHostname.c_str()))
    {
        DEBUG_PRINTF("mDNS responder started: %s.local\n", deviceHostname.c_str());
        MDNS.addService("http", "tcp", 80);
    }
    else
    {
        DEBUG_PRINTLN("mDNS responder failed to start");
    }
}
String sanitizeLocation(const String &location)
{
    String sanitized = location;
    sanitized.toLowerCase();
    for (size_t i = 0; i < sanitized.length(); ++i)
    {
        char &c = sanitized[i];
        if (!isalnum(c))
            c = '-'; // Use hyphen for non-alphanumeric
    }
    // Remove leading/trailing hyphens
    while (sanitized.startsWith("-"))
        sanitized.remove(0, 1);
    while (sanitized.endsWith("-"))
        sanitized.remove(sanitized.length() - 1, 1);
    return sanitized;
}