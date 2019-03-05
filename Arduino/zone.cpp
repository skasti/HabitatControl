//#define HEATER_MOCK
//#define RAIN_MOCK

#include "zone.h"
#include <EEPROM.h>
#include "utils.h"
#include "relays.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Uncomment this to log to serial1
//#define DEBUG true

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

void Zone::setup(int newDHTPin, int newUVPin, int8_t newHeaterRelay, int8_t newRainRelay)
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
    for (int i = 0; i < 24; i++)
    {
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

    if (config.uvPin > 0)
    {
        pinMode(config.uvPin, INPUT);
        digitalWrite(config.uvPin, HIGH);

        if (analogRead(config.uvPin) < 900)
        {
            uvEnabled = true;
            digitalWrite(config.uvPin, LOW);
        }
    }
}

void Zone::saveToEEPROM()
{
    EEPROM.put(getConfigStartLocation(), config);
    EEPROM.put(getHistoryStartLocation(), history);

    EEPROM.update(eepromLocation, 128);
}

void Zone::updateSensors(int hour, int minute, int32_t deltams, int refLevel)
{
    #ifdef DEBUG
    Serial.println("Updating sensors");
    #endif

    if (config.dhtPin > 0)
    {
        int newTemp = dht.getTemperature();
        int newHumidity = dht.getHumidity();

        if (newTemp > 0 && newHumidity > 0)
        {
    #ifndef HEATER_MOCK
            temp = newTemp;
    #else
            if (heating)
                temp += 1;
            else if (temp > 0)
                temp -= 1;
    #endif

            history.temp[hour] = temp;

            if (history.minTemp == 0 || temp < history.minTemp)
                history.minTemp = temp;

            if (history.maxTemp == 0 || temp > history.maxTemp)
                history.maxTemp = temp;

    #ifndef RAIN_MOCK
            humidity = newHumidity;
    #else
            if (raining)
                humidity += 1;
            else if (humidity > 15)
                humidity -= 1;
    #endif
            history.humidity[hour] = humidity;

            if (history.minHumidity == 0 || humidity < history.minHumidity)
                history.minHumidity = humidity;

            if (history.maxHumidity == 0 || humidity > history.maxHumidity)
                history.maxHumidity = humidity;
        }
    }

    if (uvEnabled && config.uvPin > 0)
    {
        uvi = readUVI(config.uvPin, refLevel);
        uvis += ((uvi * deltams) / 1000);

        history.uvi[hour] = uvi;

        if (history.maxUVI == 0 || uvi > history.maxUVI)
            history.maxUVI = uvi;
    }
}

void Zone::stopHeating()
{
    setRelay(config.heaterRelay, LOW);
    heating = false;
    heatCooldown = heatCooldownTime;
    #ifdef DEBUG
    Serial.println("Stopped heating");
    #endif
}

void Zone::startHeating()
{
    // Abort if no heater is configured
    if (config.heaterRelay < 0)
        return;

    // Abort if already heating
    if (heating)
        return;
    
    // Abort if we have a cooldown
    if (heatCooldown > 0)
        return;

    setRelay(config.heaterRelay, HIGH);
    heating = true;
    heatTime = 0;

    #ifdef DEBUG
    Serial.println("Started heating");
    #endif
}

void Zone::stopRaining()
{
    setRelay(config.rainRelay, LOW);
    raining = false;
    rainCooldown = rainCooldownTime;

    #ifdef DEBUG
    Serial.println("Stopped raining");
    #endif
}

void Zone::startRaining()
{
    // Abort if no rain-system is configured
    if (config.rainRelay < 0)
        return;

    // Abort if already raining
    if (raining)
        return;
    
    // Abort if we have a cooldown
    if (rainCooldown > 0)
        return;

    setRelay(config.rainRelay, HIGH);
    raining = true;
    rainTime = 0;

    #ifdef DEBUG
    Serial.println("Started raining");
    #endif
}

void Zone::update(int hour, int minute, long time, int32_t deltams, int refLevel)
{
    uint8_t tempTarget = config.tempTargets[hour];

    if (time >= nextSample) 
    {
        updateSensors(hour, minute, deltams, refLevel);
        nextSample = time + sampleInterval;
    }

    if (heating)
    {
        heatTime += deltams;

        if (heatTime > heatTimeLimit)
        {
            #ifdef DEBUG
            Serial.println("Heater timelimit");
            #endif
            stopHeating();
        }
        else if (temp > tempTarget)
        {
            #ifdef DEBUG
            Serial.println("Heater reached temp target");
            #endif
            stopHeating();
        }
    }
    else if (heatCooldown > 0) 
    {
        heatCooldown -= deltams;
    }

    if (raining)
    {
        rainTime += deltams;

        if (rainTime > rainTimeLimit)
        {
            #ifdef DEBUG
            Serial.println("Rain system timelimit");
            #endif
            stopRaining();            
            
        }
        else if (humidity > config.humidityTarget)
        {
            #ifdef DEBUG
            Serial.println("Rain system reach humidity");
            #endif
            stopRaining();
        }
    }
    else if (rainCooldown > 0) 
    {
        rainCooldown -= deltams;
    }

    if (temp > 0 && humidity > 0)
    {
        if (temp < tempTarget - lowTempThreshold)
        {
            startHeating();
        }

        if (humidity < config.humidityTarget - lowHumidityThreshold)
        {
            startRaining();
        }
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

void Zone::setDisplay(NextionDisplay *newDisplay)
{
    display = newDisplay;
}

void Zone::updateDisplayOverview()
{
    display->sendIndexValue('s', "Temp", zoneIndex, temp);
    display->sendIndexValue('s', "Humidity", zoneIndex, humidity);
    display->sendIndexValue('s', "UV", zoneIndex, uvi);
    display->sendIndexValue('s', "UVs", zoneIndex, uvi);

    if (config.heaterRelay >= 0)
        display->sendIndexValue(relayPrefix, config.heaterRelay, relayState[config.heaterRelay]);

    if (config.rainRelay >= 0)
        display->sendIndexValue(relayPrefix, config.rainRelay, relayState[config.rainRelay]);
}

void Zone::clearTempHistory()
{
    for (int i = 0; i < 24; i++)
        history.temp[i] = 0;

    saveToEEPROM();
}

void Zone::clearHumidityHistory()
{
    for (int i = 0; i < 24; i++)
        history.humidity[i] = 0;

    saveToEEPROM();
}

void Zone::clearUVIHistory()
{
    for (int i = 0; i < 24; i++)
        history.uvi[i] = 0;

    saveToEEPROM();
}