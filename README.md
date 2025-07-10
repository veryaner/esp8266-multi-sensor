# ESP8266 Multi-Sensor Network with OTA and WiFi Hotspot

A comprehensive ESP8266 project that monitors multiple sensors (temperature, humidity, motion, light, radar presence) with Over-The-Air (OTA) updates, WiFi hotspot configuration, MQTT integration, relay control, and a modern web dashboard.

## Features

- 🌡️ **DHT11/DHT22 Temperature & Humidity Sensor** - Real-time monitoring with error handling
- 👁️ **PIR Motion Sensor** - Motion detection with interrupt handling and LED feedback
- ☀️ **TSL2561 Light Sensor** - Digital light level monitoring (I2C) with auto-range
- 📡 **LD2410 Radar Sensor** - Advanced presence detection with distance sensing
- 🔌 **Relay Control** - Digital output control with MQTT integration
- 📶 **WiFi Hotspot Configuration** - Easy WiFi setup when no network available
- 🔄 **OTA Updates** - Over-the-air firmware updates via web interface
- 📊 **Real-time Dashboard** - Modern responsive web interface with live sensor data
- 🔧 **System Controls** - Restart, WiFi reset, sensor reset, and system monitoring
- 📱 **Responsive Design** - Works on desktop and mobile devices
- 📡 **MQTT Integration** - Publish sensor data to Mosquitto broker with location-based topics
- 🔄 **Individual Sensor Handling** - Auto-detect and handle sensor connections/disconnections
- 🚨 **Built-in LED Indicator** - Visual feedback for motion detection
- ⚙️ **Configuration Management** - Web-based configuration editor with export/import
- 🎛️ **Sensorless Mode** - Operate without sensors for testing and debugging
- 🔍 **Comprehensive Debug System** - Granular debug controls for troubleshooting

## Hardware Requirements

- **ESP8266 NodeMCU v2** (or compatible ESP8266 board)
- **DHT11/DHT22 Temperature & Humidity Sensor**
- **PIR Motion Sensor** (HC-SR501 or similar)
- **TSL2561 Light Sensor** (I2C digital light sensor)
- **LD2410 Radar Sensor** (24GHz presence detection)
- **Relay Module** (5V relay for digital output control)
- **Breadboard and jumper wires**
- **USB cable** for programming and power

## Wiring Diagram

### ESP8266 NodeMCU v2 Pinout
```
┌─────────────────────────────────────────┐
│  NodeMCU v2                             │
│                                         │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐       │
│  │ 3V3 │ │ GND │ │ D0  │ │ D1  │ ← SCL │
│  └─────┘ └─────┘ └─────┘ └─────┘       │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐       │
│  │ D2  │ │ D3  │ │ D4  │ │ D5  │ ← DHT │
│  └─────┘ └─────┘ └─────┘ └─────┘       │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐       │
│  │ D6  │ │ D7  │ │ D8  │ │ RX  │ ← PIR │
│  └─────┘ └─────┘ └─────┘ └─────┘       │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐       │
│  │ TX  │ │ A0  │ │ 5V  │ │ GND │       │
│  └─────┘ └─────┘ └─────┘ └─────┘       │
└─────────────────────────────────────────┘
```

### Sensor Connections

#### DHT11/DHT22 Sensor (Temperature & Humidity)
```
ESP8266 NodeMCU    DHT11/DHT22 Sensor
┌─────────────┐    ┌─────────────┐
│ GPIO14 (D5) ├────┤ Data        │
│ 3.3V        ├────┤ VCC         │
│ GND         ├────┤ GND         │
└─────────────┘    └─────────────┘
```

#### PIR Motion Sensor
```
ESP8266 NodeMCU    PIR Sensor
┌─────────────┐    ┌─────────────┐
│ GPIO16 (D0) ├────┤ Signal      │
│ 3.3V        ├────┤ VCC         │
│ GND         ├────┤ GND         │
└─────────────┘    └─────────────┘
```

#### TSL2561 Light Sensor (I2C)
```
ESP8266 NodeMCU    TSL2561 Sensor
┌─────────────┐    ┌─────────────┐
│ GPIO4 (D2)  ├────┤ SDA         │
│ GPIO5 (D1)  ├────┤ SCL         │
│ 3.3V        ├────┤ VCC         │
│ GND         ├────┤ GND         │
└─────────────┘    └─────────────┘
```

#### LD2410 Radar Sensor (UART)
```
ESP8266 NodeMCU    LD2410 Radar
┌─────────────┐    ┌─────────────┐
│ GPIO12 (D6) ├────┤ RX          │
│ GPIO13 (D7) ├────┤ TX          │
│ 3.3V        ├────┤ VCC         │
│ GND         ├────┤ GND         │
└─────────────┘    └─────────────┘
```

#### Relay Module
```
ESP8266 NodeMCU    Relay Module
┌─────────────┐    ┌─────────────┐
│ GPIO5 (D1)  ├────┤ IN          │
│ 5V          ├────┤ VCC         │
│ GND         ├────┤ GND         │
└─────────────┘    └─────────────┘
```

#### Built-in LED
- **No external wiring required**
- **Connected to GPIO2 (D4)** internally
- **Blinks when motion is detected**

## Data Structures

### ConfigData Structure
```cpp
struct ConfigData {
    char mqtt_broker[32];        // MQTT broker IP address
    char mqtt_username[32];      // MQTT username
    char mqtt_password[32];      // MQTT password
    char location[32];           // Device location (topic prefix)
    int mqtt_port;              // MQTT port (default: 1883)
    bool mqtt_enabled;          // Enable/disable MQTT
    bool sensorless_mode;       // Operate without sensors
    bool use_radar;             // Use radar instead of PIR
};
```

### SensorData Structure
```cpp
struct SensorData {
    float temperature;           // Temperature in °C
    float humidity;             // Humidity in %
    bool motion;                // Motion detected (PIR)
    float lux;                  // Light level in lux
    unsigned long timestamp;    // Last sensor read timestamp
    
    // Sensor availability flags
    bool pir_available;         // PIR sensor available
    int pir_error_count;        // PIR error counter
    bool dht_available;         // DHT sensor available
    int dht_error_count;        // DHT error counter
    bool tsl_available;         // TSL2561 sensor available
    int tsl_error_count;        // TSL2561 error counter
    bool radar_available;       // LD2410 radar available
    int radar_error_count;      // Radar error counter
    bool radar_presence;        // Radar presence detected
};
```

## Core Functions

### Sensor Management Functions
```cpp
// Main sensor functions
void setupAllSensors();         // Initialize all enabled sensors
void readAllSensors();          // Read data from all sensors
void printSensorData();         // Print sensor data to serial
void printInitialSensorData();  // Print initial sensor status

// Individual sensor functions
void setupDHT();               // Initialize DHT sensor
void readDHT();                // Read temperature and humidity
void setupTSL2561();           // Initialize TSL2561 sensor
void readTSL2561();            // Read light level
void setupPIR();               // Initialize PIR sensor
void readPIR();                // Read motion state
void setupLD2410();            // Initialize LD2410 radar
void readLD2410();             // Read radar presence
```

### Communication Functions
```cpp
// MQTT functions
void setupMQTT();              // Initialize MQTT client
bool connectMQTT();            // Connect to MQTT broker
void publishSensorData();      // Publish all sensor data
void publishRelayState();      // Publish relay state
void mqttCallback();           // Handle MQTT messages
String getTopicWithLocation(); // Get topic with location prefix

// WiFi functions
void setupWiFi();              // Initialize WiFi and hotspot
void resetWiFi();              // Reset WiFi settings

// Web server functions
void setupWebServer();         // Initialize web server
void handleClient();           // Handle web client requests
```

### Actuator Functions
```cpp
// Relay control functions
void relayInit();              // Initialize relay pin
void setRelayState(bool state); // Set relay on/off
bool getRelayState();          // Get current relay state
int getRelayPin();             // Get relay pin number
```

### Configuration Functions
```cpp
// Configuration management
void loadConfig();             // Load config from EEPROM
void saveConfig();             // Save config to EEPROM
void printFullConfig();        // Print complete configuration
void resetConfig();            // Reset to default config
```

### OTA Functions
```cpp
void setupOTA();               // Initialize OTA updates
void checkForOTA();            // Check for available updates
```

## API Endpoints

### Sensor Data Endpoints

#### GET `/api/sensors`
Returns comprehensive sensor data:
```json
{
  "temperature": 23.5,
  "humidity": 45.2,
  "motion": false,
  "luminescence": 512,
  "timestamp": 1234567890,
  "uptime": 3600000,
  "wifi_rssi": -45,
  "free_heap": 25600,
  "sensorless_mode": false,
  "motion_source": "radar",
  "dht11_available": true,
  "dht11_errors": 0,
  "tsl2561_available": true,
  "tsl2561_errors": 0,
  "pir_available": true,
  "pir_errors": 0,
  "radar_presence": false,
  "relay_state": false,
  "relay_pin": 5
}
```

#### GET `/debug/sensors`
Returns simplified sensor data for debugging:
```json
{
  "test": "sensor_data",
  "free_heap": 25600,
  "uptime": 3600000,
  "temperature": 23.5,
  "humidity": 45.2,
  "motion": false,
  "luminescence": 512,
  "timestamp": 1234567890,
  "dht_available": true,
  "tsl_available": true,
  "pir_available": true,
  "radar_available": true
}
```

#### GET `/api/sensor-config`
Returns compile-time sensor configuration:
```json
{
  "use_dht": true,
  "use_tsl2561": true,
  "use_pir": true,
  "use_ld2410": true,
  "use_relay": true
}
```

### Relay Control Endpoints

#### GET `/api/relay`
Get current relay state:
```json
{
  "state": false,
  "pin": 5
}
```

#### POST `/api/relay`
Control relay state:
```json
{
  "state": true  // or false, omit to toggle
}
```

### System Control Endpoints

#### POST `/api/restart`
Restarts the device.

#### POST `/api/wifi/reset`
Resets WiFi settings and creates hotspot.

#### POST `/api/factory-reset`
Performs a complete factory reset (clears EEPROM and WiFi settings).

### Sensor Management Endpoints

#### POST `/api/sensors/reset`
Resets all sensor error counters and marks sensors as available.

#### POST `/api/sensors/dht11/reset`
Resets DHT11 sensor error counter and marks as available.

#### POST `/api/sensors/tsl2561/reset`
Resets TSL2561 sensor error counter and marks as available.

#### POST `/api/sensors/pir/reset`
Resets PIR sensor error counter and marks as available.

### Mode Control Endpoints

#### POST `/api/sensorless`
Toggle sensorless mode:
```json
{
  "enabled": true  // or false
}
```

#### POST `/api/debug`
Toggle debug mode (requires recompilation):
```json
{
  "enabled": true  // or false
}
```

### Configuration Endpoints

#### GET `/api/config`
Returns current MQTT configuration:
```json
{
  "mqtt_broker": "192.168.1.100",
  "mqtt_port": 1883,
  "mqtt_username": "",
  "location": "sensors",
  "mqtt_enabled": true,
  "sensorless_mode": false,
  "use_radar": true,
  "mqtt_connected": true,
  "mqtt_client_id": "ESP8266_Sensor",
  "mqtt_topics": {
    "temperature": "sensors/temperature",
    "humidity": "sensors/humidity",
    "motion": "sensors/motion",
    "luminescence": "sensors/luminescence",
    "all": "sensors/all"
  }
}
```

#### GET `/api/config/full`
Returns complete system configuration including pin assignments and timing.

#### GET `/api/config/export`
Downloads configuration as JSON file.

#### POST `/api/config/print`
Prints full configuration to serial monitor.

#### POST `/api/config`
Update configuration:
```json
{
  "mqtt_broker": "192.168.1.100",
  "mqtt_port": 1883,
  "mqtt_username": "user",
  "mqtt_password": "pass",
  "location": "living_room",
  "mqtt_enabled": true,
  "sensorless_mode": false,
  "use_radar": true
}
```

### Test Endpoints

#### GET `/test`
Simple test endpoint:
```
ESP8266 is working! Free heap: 25600 bytes
```

#### GET `/health`
Health check endpoint:
```json
{
  "status": "ok",
  "uptime": 3600000,
  "free_heap": 25600,
  "wifi_rssi": -45,
  "ip": "192.168.1.100"
}
```

#### GET `/simple`
Ultra simple test endpoint:
```
Simple endpoint works!
```

#### GET `/api/test`
API test endpoint:
```json
{
  "status": "ok",
  "message": "API is working"
}
```

## MQTT Integration

### MQTT Configuration
- **Broker**: Configurable IP address (default: 192.168.1.100)
- **Port**: Configurable (default: 1883)
- **Authentication**: Optional username/password
- **Client ID**: Auto-generated with random suffix
- **Keep Alive**: 60 seconds
- **Reconnect Interval**: 5 seconds
- **Publish Interval**: 10 seconds

### MQTT Topics
All topics are prefixed with the configured location:

#### Individual Sensor Topics
- `{location}/temperature` - Temperature in °C
- `{location}/humidity` - Humidity in %
- `{location}/motion` - Motion detection (1 = motion, 0 = no motion)
- `{location}/luminescence` - Light level in lux
- `{location}/radar` - Radar presence (1 = presence, 0 = no presence)
- `{location}/relay` - Relay state and configuration
- `{location}/status` - Device status ("online"/"offline")

#### Combined Data Topic
- `{location}/all` - All sensor data as JSON

#### Relay Control Topic
- `{location}/relay` - Relay commands and state updates

### MQTT Message Examples

#### Individual Topics
```
living_room/temperature: "23.50"
bedroom/humidity: "45.20"
kitchen/motion: "1"
garage/luminescence: "512"
office/radar: "1"
```

#### Relay State Topic
```json
{
  "state": true,
  "pin": 5,
  "timestamp": 1234567890,
  "location": "living_room"
}
```

#### All Data Topic
```json
{
  "temperature": 23.5,
  "humidity": 45.2,
  "motion": false,
  "luminescence": 512,
  "radar_presence": false,
  "relay_state": false,
  "relay_pin": 5,
  "timestamp": 1234567890,
  "location": "living_room",
  "uptime": 3600000,
  "free_heap": 25600,
  "wifi_rssi": -45
}
```

#### Relay Commands
```json
{
  "command": "toggle"
}
```
or
```json
{
  "command": "set",
  "state": true
}
```

## Operating Modes

### 1. Normal Sensor Mode (Default)
- All sensors active and monitored
- Real-time data collection and publishing
- Full web dashboard functionality
- MQTT integration enabled

### 2. Sensorless Mode
- Sensors disabled, WiFi and MQTT remain active
- Useful for testing connectivity without sensors
- Can be toggled via API or web interface
- Reduces power consumption

### 3. Individual Sensor Modes
Each sensor operates independently with automatic error detection:
- **DHT11/DHT22**: Temperature and humidity
- **TSL2561**: Light level monitoring
- **PIR**: Motion detection with LED feedback
- **LD2410**: Advanced presence detection
- **Relay**: Digital output control

### 4. Auto-Recovery Mode
- Monitors consecutive sensor read failures
- Automatically marks sensors as unavailable after 3 failures
- Attempts to re-detect sensors periodically
- Manual reset available via API endpoints

## Debug System

### Master Debug Mode
```cpp
#define DEBUG_MODE true        // Enable all debug output
#define DEBUG_SENSORS true     // Sensor debug messages
#define DEBUG_WEB_SERVER true  // Web server debug messages
#define DEBUG_MQTT true        // MQTT debug messages
#define DEBUG_MEMORY true      // Memory usage debug messages
#define DEBUG_WIFI true        // WiFi debug messages
#define DEBUG_OTA true         // OTA debug messages
```

### Selective Debug Mode
```cpp
#define DEBUG_MODE true
#define DEBUG_SENSORS true      // Only sensor issues
#define DEBUG_WEB_SERVER true   // Only web server issues
#define DEBUG_MQTT false        // Keep MQTT quiet
#define DEBUG_MEMORY false      // Keep memory quiet
#define DEBUG_WIFI false        // Keep WiFi quiet
#define DEBUG_OTA false         // Keep OTA quiet
```

### Debug API Endpoints

#### Enable/Disable Debug Mode
```bash
# Enable debug mode (requires recompilation)
curl -X POST http://[device-ip]/api/debug \
  -H "Content-Type: application/json" \
  -d '{"enabled": true}'

# Disable debug mode (requires recompilation)
curl -X POST http://[device-ip]/api/debug \
  -H "Content-Type: application/json" \
  -d '{"enabled": false}'
```

## Configuration Parameters

### Pin Configuration
```cpp
#define DHT_PIN 14         // D5 on NodeMCU (GPIO14)
#define PIR_PIN 16         // D0 on NodeMCU (GPIO16)
#define LED_PIN 2          // Built-in LED (GPIO2/D4)
#define RELAY_PIN 5        // D1 on NodeMCU (GPIO5)
#define SDA_PIN 4          // D2 on NodeMCU (GPIO4)
#define SCL_PIN 5          // D1 on NodeMCU (GPIO5)
#define LD2410_RX_PIN 12   // D6 on NodeMCU (GPIO12)
#define LD2410_TX_PIN 13   // D7 on NodeMCU (GPIO13)
```

### Timing Configuration
```cpp
#define SENSOR_READ_INTERVAL 5000        // 5 seconds
#define MQTT_PUBLISH_INTERVAL 10000      // 10 seconds
#define OTA_CHECK_INTERVAL 300000        // 5 minutes
#define PIR_COOLDOWN 10000               // 10 seconds
#define WATCHDOG_TIMEOUT 30000           // 30 seconds
```

### WiFi Configuration
```cpp
#define WIFI_SSID "ESP8266_Sensor_Network"
#define WIFI_PASSWORD "12345678"
#define AP_IP "192.168.4.1"
#define DEVICE_HOSTNAME "esp8266-sensor"
#define WIFI_CONFIG_TIMEOUT 180
```

### MQTT Configuration
```cpp
#define MQTT_BROKER "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_CLIENT_ID "ESP8266_Sensor"
#define MQTT_KEEPALIVE 60
#define MQTT_RECONNECT_INTERVAL 5000
```

### Sensor Configuration
```cpp
#define DHT_TYPE DHT11
#define PIR_SENSITIVITY HIGH
#define LUMINESCENCE_DARK_THRESHOLD 200
#define LUMINESCENCE_BRIGHT_THRESHOLD 800
#define SENSOR_ERROR_THRESHOLD 3
#define LD2410_BAUD_RATE_2 115200
```

### Feature Flags
```cpp
#define USE_DHT 1
#define USE_TSL2561 1
#define USE_PIR 1
#define USE_LD2410 1
#define USE_RELAY 1
#define ENABLE_OTA true
#define ENABLE_WIFI_MANAGER true
#define ENABLE_NTP true
#define ENABLE_LITTLEFS true
```

## Software Setup

### Prerequisites
1. **PlatformIO IDE** (VS Code extension or standalone)
2. **ESP8266 RTOS SDK** (automatically installed via platformio.ini)

### Installation
1. **Clone or download this project**
   ```bash
   git clone <repository-url>
   cd MultiSensor
   ```

2. **Open in PlatformIO**
   - Open the project folder in VS Code with PlatformIO extension
   - Or use PlatformIO CLI: `pio project init --board nodemcuv2`

3. **Build and Upload**
   ```bash
   # Build the project
   pio run
   
   # Upload to device
   pio run --target upload
   
   # Monitor serial output
   pio device monitor
   ```

## Usage

### First Time Setup
1. **Power on the ESP8266**
   - Device attempts to connect to saved WiFi credentials
   - If no WiFi configured, creates hotspot

2. **Connect to Hotspot** (if needed)
   - SSID: `ESP8266_Sensor_Network`
   - Password: `12345678`
   - Connect your phone/computer to this network

3. **Configure WiFi and MQTT**
   - Open browser and go to `192.168.4.1`
   - Configure WiFi network and MQTT settings
   - Device connects and remembers settings

### Accessing the Dashboard
1. **Find the device IP**
   - Check serial monitor for IP address
   - Or scan your network for the device

2. **Open the dashboard**
   - Navigate to `http://[device-ip]` in your browser
   - Example: `http://192.168.1.100`

### Dashboard Features
- **Real-time Sensor Data**: All sensor readings with live updates
- **System Status**: WiFi signal, free memory, uptime
- **Sensor Status**: Individual sensor availability and error counts
- **Relay Control**: On/off control with visual feedback
- **Configuration Editor**: Web-based config management
- **System Controls**: Restart, WiFi reset, sensor reset, OTA update

### OTA Updates
1. **Build new firmware**
   ```bash
   pio run
   ```

2. **Access OTA page**
   - Go to `http://[device-ip]/update`
   - Or click "OTA Update" button in dashboard
   - Select the `.bin` file from `.pio/build/nodemcuv2/firmware.bin`
   - Click "Upload Firmware"

## Examples

### Python API Client
See `examples/api_example.py` for a complete Python client demonstrating:
- Sensor data retrieval
- Device control (restart, WiFi reset)
- Configuration management
- Error handling and retry logic

### MQTT Subscriber
See `examples/mqtt_subscriber.py` for a Python MQTT subscriber that:
- Connects to MQTT broker
- Subscribes to all sensor topics
- Displays real-time sensor data
- Handles JSON payloads and individual topics

## Troubleshooting

### Common Issues

#### MQTT Connection Errors (rc=-2)
- Check broker IP address and port
- Verify network connectivity
- Test with public MQTT brokers (test.mosquitto.org:1883)
- Check firewall settings

#### Sensor Not Detected
- Verify wiring connections
- Check power supply (3.3V for sensors)
- Use sensor reset API endpoints
- Check serial monitor for error messages

#### WiFi Connection Issues
- Use WiFi reset to create hotspot
- Check WiFi credentials
- Verify signal strength
- Try different WiFi networks

#### Memory Issues
- Monitor free heap via `/health` endpoint
- Reduce debug output if needed
- Check for memory leaks in custom code
- Consider sensorless mode for testing

### Debug Output Examples

#### Clean Mode (DEBUG_MODE = false)
```
=== ESP8266 Sensor Network ===
Setup complete!
```

#### Full Debug Mode (DEBUG_MODE = true)
```
=== ESP8266 Sensor Network ===
Starting setup...
Free heap at start: 45678 bytes
Initializing EEPROM...
EEPROM initialized
Loading configuration...
Configuration loaded:
  MQTT Broker: 192.168.1.100:1883
  Location: sensors
  MQTT Enabled: Yes
  Sensorless Mode: No
Initializing LittleFS...
LittleFS available - setting up static file serving
Setting up sensors...
I2C initialized on SDA:4, SCL:5
Initializing DHT11 on pin 14...
DHT11 sensor initialized
PIR sensor initialized on pin 16
Initializing TSL2561 light sensor...
TSL2561 light sensor initialized
TSL2561 Sensor: TSL2561
Driver Version: 1
Unique ID: 12345
Max Value: 40000.0 lux
Min Value: 0.1 lux
Resolution: 0.1 lux
Performing initial sensor read...
=== Initial Sensor Status ===
DHT11 (Temp/Humidity): Available
TSL2561 (Light): Available
PIR (Motion): Available
=============================
Setting up WiFi...
WiFi connected!
IP address: 192.168.1.100
Setting up web server...
LittleFS available - setting up static file serving
Web server started
Free heap after web server setup: 23456 bytes
Setting up MQTT...
MQTT broker: 192.168.1.100:1883
Location: sensors
Connecting to MQTT broker... connected!
Setting up OTA...
PIR interrupt management enabled
Setup complete!
=== Sensor Data ===
Temperature: 22.10°C
Humidity: 66.00%
Motion: NO
Luminescence: 512.00 lux
Free Heap: 23456 bytes
==================
API /api/sensors requested
Free heap before API call: 23456 bytes
API response length: 245 bytes
API response: {"temperature":22.10,"humidity":66.00,"motion":false,"luminescence":512.00,"timestamp":1234567890,"uptime":3600000,"wifi_rssi":-45,"free_heap":23456,"sensorless_mode":false,"dht11_available":true,"dht11_errors":0,"tsl2561_available":true,"tsl2561_errors":0,"pir_available":true,"pir_errors":0}
Free heap after API call: 23456 bytes
Publishing sensor data to MQTT...
Sensor data published to MQTT
Debug - Free heap: 23456 bytes, Uptime: 30000 ms
```

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Support

For support and questions:
1. Check the troubleshooting section above
2. Review the debug output and serial monitor
3. Test with the provided examples
4. Open an issue on the project repository 