#include "zone.h"
#include <EEPROM.h>
#include "utils.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

Zone::Zone(int eepromLocation)
{
    this->eepromLocation = eepromLocation;
}

int Zone::getConfigStartLocation()
{
    return eepromLocation + 1;
}

int Zone::getHistoryStartLocation()
{
    return getConfigStartLocation() + sizeof(ZoneConfig);
}

int Zone::getTempHistoryLocation(int hour)
{
    return getHistoryStartLocation() + hour;
}

int Zone::getHumidityHistoryLocation(int hour)
{
    return getTempHistoryLocation(hour) + 24;
}

int Zone::getUVISHistoryLocation(int hour)
{
    return getHumidityHistoryLocation(hour) + 24;
}

void Zone::setup (int newDHTPin, int newUVPin, uint8_t newHeaterRelay, uint8_t newRainRelay)
{
    config.dhtPin = newDHTPin;
    config.uvPin = newUVPin;
    config.heaterRelay = newHeaterRelay;
    config.rainRelay = newRainRelay;

    dht.setup(config.dhtPin);

    saveToEEPROM();
}

void Zone::configureTargets(uint8_t newTempTarget, uint8_t newHumidityTarget)
{
    config.tempTarget = newTempTarget;
    config.humidityTarget = newHumidityTarget;

    saveToEEPROM();
}

bool Zone::loadFromEEPROM()
{
    uint8_t eepromState = EEPROM.read(eepromLocation);

    //Uninitialized eeprom locations are 255
    //This means we cannot load
    if (eepromState == 255)
        return false;

    EEPROM.get(getConfigStartLocation(), config);
    EEPROM.get(getHistoryStartLocation(), history);

    if (config.dhtPin > 0)
        dht.setup(config.dhtPin);

    return true;
}

void Zone::saveToEEPROM()
{
    EEPROM.put(getConfigStartLocation(), config);
    EEPROM.put(getHistoryStartLocation(),history);

    EEPROM.update(eepromLocation,128);
}

void Zone::update(int hour, int minute, int deltams, int refLevel)
{
    if (config.dhtPin > 0)
    {
        temp = dht.getTemperature();
        humidityTarget = dht.getHumidity();
    }

    if (config.uvPin > 0)
    {
        uvi = readUVI(config.uvPin, refLevel);
        
    }
}

ZoneHistory Zone::getHistory() 
{
    return history;
}