#ifndef ZONE_H
#define ZONE_H

#include "DHT.h"
#include "nextionDisplay.h"
#include <inttypes.h>

const int32_t heatTimeLimit = 10*60*1000; //10 Minutes
const uint16_t rainTimeLimit = 5000; // 5 Seconds

const int32_t heatCooldownTime = 5*60*1000; //5 minutes
const int32_t rainCooldownTime = 60*60*1000; //1 Hour

struct ZoneHistory
{
    uint8_t temp[24];
    uint8_t humidity[24];
    uint8_t uvi[24];
    uint8_t minTemp = 0;
    uint8_t maxTemp = 0;
    uint8_t minHumidity = 0;
    uint8_t maxHumidity = 0;
    uint8_t maxUVI = 0;
};

struct ZoneConfig
{
    int dhtPin, uvPin;
    int8_t heaterRelay = 0;
    int8_t rainRelay = 0;
    uint8_t tempTargets[24] = {
        20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20,
        20, 20, 20, 20, 20, 20};
    uint8_t humidityTarget = 35;
};

const uint8_t lowTempThreshold = 2;
const uint8_t lowHumidityThreshold = 5;

class Zone
{
    uint8_t temp = 0;
    uint8_t humidity = 0;

    uint8_t uvi = 0;
    uint16_t uvis = 0;
    bool uvEnabled = false;

    bool heating, raining;
    int32_t heatTime, rainTime;
    int32_t heatCooldown, rainCooldown;

    ZoneHistory history;
    ZoneConfig config;

    DHT dht;

    int eepromLocation;
    int zoneIndex;
    NextionDisplay *display;

  private:
    int getConfigStartLocation();
    int getHistoryStartLocation();
    int getTempHistoryLocation(int hour);
    int getHumidityHistoryLocation(int hour);
    int getUVISHistoryLocation(int hour);
    void init();

  public:
    Zone(int eepromLocation, int zoneIndex);

    void setup(int newDHTPin, int newUVPin, int8_t newHeaterRelay, int8_t newRainRelay);

    void configureTargets(uint8_t newTempTargets[], uint8_t newHumidityTarget);

    bool loadFromEEPROM();
    void saveToEEPROM();

    void update(int hour, int minute, int deltams, int refLevel);

    ZoneHistory getHistory();
    ZoneConfig getConfig();

    int getTemp();
    int getHumidity();
    int getUVI();
    int getUVIS();

    void setDisplay(NextionDisplay *newDisplay);
    void updateDisplayOverview();
};

#endif