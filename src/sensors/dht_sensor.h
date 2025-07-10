#pragma once
#include <DHT.h>
extern bool dhtAvailable;
extern int dhtErrorCount;
extern unsigned long lastDhtError;
extern DHT dht ;
void setupDHT();
void readDHT(); 