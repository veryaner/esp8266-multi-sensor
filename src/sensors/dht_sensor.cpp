#include <DHT.h>
#include "config.h"
#include "model/data_structs.h"
#include "sensors/dht_sensor.h"
#include "debug/debug_macros.h"

extern long currentTime;
bool dhtAvailable = false;
int dhtErrorCount  = 0;
unsigned long lastDhtError = 0;
DHT dht(DHT_PIN, DHT11);

void setupDHT() {
    SENSOR_DEBUG_PRINTF("Initializing DHT11 on pin %d...\n", DHT_PIN);
    dht.begin();
    delay(1000); // Give DHT sensor time to stabilize
    ESP.wdtFeed();
    SENSOR_DEBUG_PRINTLN("DHT11 sensor initialized");
}

void readDHT() {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum))
    {
        sensorData.temperature = temp;
        sensorData.humidity = hum;
        // Reset error counter on successful read
        if (dhtErrorCount > 0)
        {
            SENSOR_DEBUG_PRINTF("DHT11 working again. Error count reset from %d\n", dhtErrorCount);
            dhtErrorCount = 0;
        }
    }
    else
    {
        SENSOR_DEBUG_PRINTLN("DHT11 sensor read failed");
        dhtErrorCount++;
        lastDhtError = currentTime;

        if (dhtErrorCount >= SENSOR_ERROR_THRESHOLD)
        {
            SENSOR_DEBUG_PRINTF("DHT11 sensor failed %d times. Marking as unavailable.\n", dhtErrorCount);
            dhtAvailable = false;
            sensorData.temperature = 0.0;
            sensorData.humidity = 0.0;
        }
    }
} 