#pragma once

// Debug macros
#if DEBUG_MODE
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
#endif

// Category-specific debug macros
#if DEBUG_SENSORS
#define SENSOR_DEBUG_PRINT(x) Serial.print(x)
#define SENSOR_DEBUG_PRINTLN(x) Serial.println(x)
#define SENSOR_DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define SENSOR_DEBUG_PRINT(x)
#define SENSOR_DEBUG_PRINTLN(x)
#define SENSOR_DEBUG_PRINTF(fmt, ...)
#endif

#if DEBUG_WEB_SERVER
#define WEB_DEBUG_PRINT(x) Serial.print(x)
#define WEB_DEBUG_PRINTLN(x) Serial.println(x)
#define WEB_DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define WEB_DEBUG_PRINT(x)
#define WEB_DEBUG_PRINTLN(x)
#define WEB_DEBUG_PRINTF(fmt, ...)
#endif

#if DEBUG_MQTT
#define MQTT_DEBUG_PRINT(x) Serial.print(x)
#define MQTT_DEBUG_PRINTLN(x) Serial.println(x)
#define MQTT_DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define MQTT_DEBUG_PRINT(x)
#define MQTT_DEBUG_PRINTLN(x)
#define MQTT_DEBUG_PRINTF(fmt, ...)
#endif

#if DEBUG_MEMORY
#define MEMORY_DEBUG_PRINT(x) Serial.print(x)
#define MEMORY_DEBUG_PRINTLN(x) Serial.println(x)
#define MEMORY_DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
#define MEMORY_DEBUG_PRINT(x)
#define MEMORY_DEBUG_PRINTLN(x)
#define MEMORY_DEBUG_PRINTF(fmt, ...)
#endif 