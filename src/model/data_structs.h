#pragma once

struct ConfigData {
    char mqtt_broker[32];
    char mqtt_username[32];
    char mqtt_password[32];
    char location[32];
    int mqtt_port;
    bool mqtt_enabled;
    bool sensorless_mode;
    bool use_radar;
};

struct SensorData {
    float temperature;
    float humidity;
    bool motion;
    float lux;
    unsigned long timestamp;
    
    bool pir_available;
    int pir_error_count;
    bool dht_available;
    int dht_error_count;
    bool tsl_available;
    int tsl_error_count;
    bool radar_available;
    int radar_error_count;
    bool radar_presence;
};

extern ConfigData config;
extern SensorData sensorData; 