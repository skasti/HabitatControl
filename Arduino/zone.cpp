#include "zone.h"
#include <EEPROM.h>
#include "utils.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

Zone::Zone(int eepromLocation, int zoneIndex)
{
    this->eepromLocation = eepromLocation;
    this->zoneIndex = zoneIndex;
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

void Zone::setup (int newDHTPin, int newUVPin, int8_t newHeaterRelay, int8_t newRainRelay)
{
    config.dhtPin = newDHTPin;
    config.uvPin = newUVPin;
    config.heaterRelay = newHeaterRelay;
    config.rainRelay = newRainRelay;

    init();

    saveToEEPROM();
}

void Zone::configureTargets(uint8_t newTempTargets[], uint8_t newHumidityTarget)
{
    for (int i = 0; i < 24; i++) {
        config.tempTargets[i] = newTempTargets[i];
    }
    
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

    init();

    return true;
}

void Zone::init() 
{
    if (config.dhtPin > 0)
        dht.setup(config.dhtPin);

    if (config.uvPin > 0) {
        pinMode(config.uvPin, INPUT);
        digitalWrite(config.uvPin, HIGH);

        if (analogRead(config.uvPin) < 1000) {
            uvEnabled = true;
            digitalWrite(config.uvPin, LOW);
        }
    }
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

        history.temp[hour] = temp;

        if (history.minTemp == 0 || temp < history.minTemp)
            history.minTemp = temp;

        if (history.maxTemp == 0 || temp > history.maxTemp)
            history.maxTemp = temp;

        if (config.heaterRelay >= 0)
        {
            uint8_t tempTarget = config.tempTargets[hour];

            if (temp < tempTarget - lowTempThreshold)
            {
                display->sendIndexValue('r',"",config.heaterRelay, 1);
            } else if (temp > tempTarget)
            {
                display->sendIndexValue('r',"",config.heaterRelay, 0);
            }
        }

        humidity = dht.getHumidity();

        history.humidity[hour] = humidity;

        if (history.minHumidity == 0 || humidity < history.minHumidity)
            history.minHumidity = humidity;

        if (history.maxHumidity == 0 || humidity > history.maxHumidity)
            history.maxHumidity = humidity;

        if (config.rainRelay >= 0)
        {
            if (humidity < config.humidityTarget - lowHumidityThreshold)
            {
                display->sendIndexValue('r',"",config.rainRelay, 1);
            } else if (humidity > config.humidityTarget)
            {
                display->sendIndexValue('r',"",config.rainRelay, 0);
            }
        }
    }

    if (uvEnabled && config.uvPin > 0)
    {
        uvi = readUVI(config.uvPin, refLevel);
        
        history.uvi[hour] = uvi;

        if (history.maxUVI == 0 || uvi > history.maxUVI)
            history.maxUVI = uvi;
    }
}

ZoneHistory Zone::getHistory() 
{
    return history;
}

ZoneConfig Zone::getConfig() 
{
    return config;
}

int Zone::getTemp() 
{
    return temp;
}

int Zone::getHumidity()
{
    return humidity;
}

int Zone::getUVI()
{
    return uvi;
}

int Zone::getUVIS()
{
    return uvis;
}

void Zone::setDisplay(NextionDisplay* newDisplay)
{
    display = newDisplay;
}

void Zone::updateDisplayOverview()
{
    display->sendIndexValue('s',"Temp", zoneIndex, temp);
    display->sendIndexValue('s',"Humidity", zoneIndex, humidity);
    display->sendIndexValue('s',"UV", zoneIndex, uvi);
    display->sendIndexValue('s',"UVs", zoneIndex, uvi);
}