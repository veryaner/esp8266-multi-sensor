#pragma once
#ifndef CONFIG_H
#define CONFIG_H
// ============================================================================
// SENSOR ENABLE FLAGS
// ============================================================================
#define USE_DHT 0
#define USE_TSL2561 0
#define USE_PIR 0
#define USE_LD2410 1
#define USE_RELAY 1

// ============================================================================
// PIN CONFIGURATION
// ============================================================================

// Sensor pins (NodeMCU v2 pinout)
#define DHT_PIN 14  // D5 on NodeMCU (GPIO14) - safer pin, avoid GPIO0
#define PIR_PIN 16  // D0 on NodeMCU (GPIO16) - safe, no SPI conflicts
#define LED_PIN 2   // Built-in LED (GPIO2/D4)
#define RELAY_PIN 5 // D1  on NodeMCU (GPIO5), change as needed

// I2C pins for TSL2561
#define SDA_PIN 4 // D2 on NodeMCU (GPIO4)
#define SCL_PIN 5 // D1 on NodeMCU (GPIO5)

// LD2401 PINS
#define LD2410_RX_PIN 12 // Example: D6
#define LD2410_TX_PIN 13 // Example: D7
#define LD2410_BAUD_RATE_2 115200

// ============================================================================
// WIFI CONFIGURATION
// ============================================================================

// Hotspot settings (when no WiFi is configured)
#define WIFI_SSID "ESP8266_Sensor_Network"
#define WIFI_PASSWORD "12345678"
#define AP_IP "192.168.4.1"
#define AP_GATEWAY "192.168.4.1"
#define AP_SUBNET "255.255.255.0"

// Device hostname (for mDNS)
#define DEVICE_HOSTNAME "esp8266-sensor"

// WiFi configuration portal timeout (seconds)
#define WIFI_CONFIG_TIMEOUT 180

// ============================================================================
// TIMING CONFIGURATION
// ============================================================================

// Sensor reading intervals (milliseconds)
#define SENSOR_READ_INTERVAL 5000 // 5 seconds
#define OTA_CHECK_INTERVAL 300000 // 5 minutes
#define PIR_COOLDOWN 10000        // 10 seconds

// Sensor error handling
#define SENSOR_ERROR_THRESHOLD 3 // Number of consecutive errors before switching to sensorless mode

// ============================================================================
// SENSOR CONFIGURATION
// ============================================================================

// DHT sensor type (DHT11 or DHT22)
#define DHT_TYPE DHT11

// PIR sensor sensitivity (adjust if needed)
#define PIR_SENSITIVITY HIGH

// Luminescence sensor calibration
#define LUMINESCENCE_DARK_THRESHOLD 200   // Below this = dark
#define LUMINESCENCE_BRIGHT_THRESHOLD 800 // Above this = bright

// ============================================================================
// DEBUG CONFIGURATION
// ============================================================================

// Master debug mode (set to false for clean logs)
#define DEBUG_MODE false

// Individual debug categories (only active when DEBUG_MODE is true)
#define DEBUG_SENSORS false     // Turn on/off sensor logs
#define DEBUG_WEB_SERVER false // Turn on/of web server debug
#define DEBUG_MQTT true       // Turn on MQTT logs for troubleshooting
#define DEBUG_MEMORY false     // Turn on/off memory logs
#define DEBUG_WIFI false       // Turn on/off WiFi logs
#define DEBUG_OTA false        // Turn on/off OTA logs

// Serial monitor baud rate
#define SERIAL_BAUD 115200

// ============================================================================
// WEB SERVER CONFIGURATION
// ============================================================================

// Web server port
#define WEB_SERVER_PORT 80

// API response timeout (milliseconds)
#define API_TIMEOUT 5000

// ============================================================================
// MQTT CONFIGURATION
// ============================================================================

// MQTT Broker settings
#define MQTT_BROKER "192.168.1.100"     // Your Mosquitto server IP
#define MQTT_PORT 1883                  // MQTT port (1883 for unencrypted, 8883 for SSL)
#define MQTT_USERNAME ""                // MQTT username (leave empty if no auth)
#define MQTT_PASSWORD ""                // MQTT password (leave empty if no auth)
#define MQTT_CLIENT_ID "ESP8266_Sensor" // Unique client ID

// MQTT Topics (will be prefixed with location)
#define MQTT_TOPIC_TEMPERATURE "temperature"
#define MQTT_TOPIC_HUMIDITY "humidity"
#define MQTT_TOPIC_MOTION "motion"
#define MQTT_TOPIC_LUMINESCENCE "luminescence"
#define MQTT_TOPIC_RADAR_PRESENCE "radar"
#define MQTT_TOPIC_STATUS "status"
#define MQTT_TOPIC_RELAY "relay"
#define MQTT_TOPIC_ALL "all"
#define DEFAULT_USE_RADAR true
// MQTT Settings
#define MQTT_KEEPALIVE 60            // Keep alive interval in seconds
#define MQTT_RECONNECT_INTERVAL 5000 // Reconnect interval in milliseconds
#define MQTT_PUBLISH_INTERVAL 10000  // Publish interval in milliseconds (10 seconds)

// ============================================================================
// OTA CONFIGURATION
// ============================================================================

// OTA update settings
#define OTA_PORT 8266
#define OTA_PASSWORD "admin"

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================

// Watchdog timeout (milliseconds)
#define WATCHDOG_TIMEOUT 30000

// Memory threshold for warnings (bytes)
#define MEMORY_WARNING_THRESHOLD 10000

// ============================================================================
// FEATURE FLAGS
// ============================================================================

// Enable/disable features
#define ENABLE_OTA true
#define ENABLE_WIFI_MANAGER true
#define ENABLE_NTP true
#define ENABLE_LITTLEFS true
#define ENABLE_MOTION_INTERRUPT true
#define ENABLE_MQTT true
#define DEFAULT_SENSORLESS_MODE false // Set to true for sensorless mode, false for sensor mode

// ============================================================================
// CALIBRATION VALUES
// ============================================================================

// Temperature offset (add/subtract from readings)
#define TEMPERATURE_OFFSET 0.0

// Humidity offset (add/subtract from readings)
#define HUMIDITY_OFFSET 0.0

// Luminescence calibration values
#define LUMINESCENCE_MIN 0
#define LUMINESCENCE_MAX 1023

#endif // CONFIG_H