#ifndef ZONE_H
#define ZONE_H

#include "DHT.h"
#include <inttypes.h>

struct ZoneHistory {
    uint8_t temp[24];
    uint8_t humidity[24];
    uint8_t uvi[24];
    uint8_t minTemp = 0;
    uint8_t maxTemp = 0;
    uint8_t minHumidity = 0;
    uint8_t maxHumidity = 0;
    uint8_t maxUVI = 0;
};

struct ZoneConfig {
    int dhtPin, uvPin;
    uint8_t heaterRelay = 0;
    uint8_t rainRelay = 0;
    uint8_t tempTarget = 25;
    uint8_t humidityTarget = 35;
};

class Zone {
    uint8_t temp;
    uint8_t humidity;
    
    uint8_t uvi;
    uint16_t uvis = 0;    

    ZoneHistory history;    
    ZoneConfig config;

    DHT dht;

    int eepromLocation;

    private:
        int getConfigStartLocation();
        int getHistoryStartLocation();
        int getTempHistoryLocation(int hour);
        int getHumidityHistoryLocation(int hour);
        int getUVISHistoryLocation(int hour);

    public:
        Zone(int eepromLocation);
        
        void setup (int newDHTPin, int newUVPin, uint8_t newHeaterRelay, uint8_t newRainRelay);

        void configureTargets(uint8_t newTempTarget, uint8_t newHumidityTarget);

        bool loadFromEEPROM();
        void saveToEEPROM();

        void update(int hour, int minute, int deltams, int refLevel);

        ZoneHistory getHistory();
};

#endif