#pragma once
#include <DHT.h>
extern int dhtErrorCount;
extern unsigned long lastDhtError;
extern DHT dht ;
void setupDHT();
void readDHT(); 