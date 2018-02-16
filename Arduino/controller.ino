#include "DHT.h"
#include "utils.h"
#include "zone.h"
#include "nextionDisplay.h"
#include "relays.h"

int8_t uvs[] = {
  A3, A2, A1
};

int8_t heaters[] = {
  -1, 0, -1
};

int8_t rain[] = {
  -1, -1, -1
};

long sampleTime = 10000;
long prevTime = 0;

Zone zone[3] = {
  Zone(0,0),
  Zone(200,1),
  Zone(400,2)
};

bool isOverview = true;

NextionDisplay display;

long time;

uint8_t hour = 0;
uint8_t minute = 0;

long nextMinute = 60000;
uint8_t nextTimePoll = 0;
uint8_t prevHour = 0;

const uint8_t timePollInterval = 6; //6 Hours

uint8_t lightOn = 10;
uint8_t lightOff = 22;
int lightRelay = 2;

void setup()
{
  display.setup();

  pinMode(A0, INPUT);

  setupRelays();

  for (int i = 0; i < 3; i++)
  {
    if (!zone[i].loadFromEEPROM())
    {
      zone[i].setup(10 + i, uvs[i], heaters[i], rain[i]);
    }
  }
}

void getDisplayCommand()
{
  if (display.hasCommand())
  {
    String command = display.getCommand();

    if (command == "T")
    {
      hour = display.getIntValue(F("overview.hour"));
      minute = display.getIntValue(F("overview.minute"));
      nextMinute = time + 60000;
      nextTimePoll = hour + timePollInterval;

      if (nextTimePoll >= 24)
        nextTimePoll -= 24;

      display.sendValue("CurrentHour",hour);
      display.sendValue("CurrentMinute",minute);
      display.sendValue("NextTimePoll",nextTimePoll);
    }

    if (command == "Z")
    {
      isOverview = false;

      int8_t zoneIndex = display.getIntValue(F("global.currentZone"));

      if (zoneIndex >= 0)
      {
        ZoneConfig config = zone[zoneIndex].getConfig();
        display.sendValue("temp", zone[zoneIndex].getTemp());
        display.sendValue("humidity", zone[zoneIndex].getHumidity());
        display.sendValue("tempTarget", config.tempTargets[hour]);
        display.sendValue("humidityTarget", config.humidityTarget);
        display.sendValue("uvi", zone[zoneIndex].getUVI());
        display.sendValue("uvis", zone[zoneIndex].getUVIS());
      }
    }
    else if (command == "TT")
    {
      int8_t zoneIndex = display.getIntValue(F("global.currentZone"));

      if (zoneIndex >= 0)
      {
        ZoneConfig config = zone[zoneIndex].getConfig();

        for (uint8_t i = 0; i < 24; i++)
        {
          display.sendIndexValue('h', i, config.tempTargets[i]);
        }
      }
    }
    else if (command == "HT")
    {
      int8_t zoneIndex = display.getIntValue(F("global.currentZone"));

      if (zoneIndex >= 0)
      {
        ZoneConfig config = zone[zoneIndex].getConfig();
        display.sendValue("target", config.humidityTarget);
        display.sendValue("display", config.humidityTarget);
      }
    }
    else if (command == "GG")
    {
      int8_t zoneIndex = display.getIntValue(F("global.currentZone"));
      int8_t currentAttribute = display.getIntValue(F("global.currentAttr"));

      if (zoneIndex >= 0)
      {
        ZoneHistory history = zone[zoneIndex].getHistory();

        for (int8_t i = 23; i >= 0; i--)
        {
          //One datapoint is only 1px, so we need to send a few to get a proper graph
          for (uint8_t j = 0; j <= 14; j++)
          {
            if (currentAttribute == 0)
              display.sendWaveFormValue(1,0, history.temp[i] * 4);
            else if (currentAttribute == 1)
              display.sendWaveFormValue(1,0, history.humidity[i] * 2);
            else if (currentAttribute == 2)
              display.sendWaveFormValue(1,0, history.uvi[i] * 10);

            if (i < hour)
              display.sendWaveFormValue(1,1, 0);
            else
              display.sendWaveFormValue(1,1, 160);
          }
        }
      }
    }
    else if (command == "CG")
    {
      int8_t zoneIndex = display.getIntValue(F("global.currentZone"));
      int8_t currentAttribute = display.getIntValue(F("global.currentAttr"));

      if (zoneIndex >= 0)
      {
        if (currentAttribute == 0)
          zone[zoneIndex].clearTempHistory();
        else if (currentAttribute == 1)
          zone[zoneIndex].clearHumidityHistory();
        else if (currentAttribute == 2)
          zone[zoneIndex].clearUVIHistory();
      }
    }
    else if (command == "STT")
    {
      int8_t zoneIndex = display.getIntValue(F("global.currentZone"));

      if (zoneIndex >= 0)
      {
        ZoneConfig config = zone[zoneIndex].getConfig();

        for (int8_t i = 0; i < 24; i++)
        {
          int8_t newValue = display.getIntValue('h', i);

          if (newValue >= 0)
            config.tempTargets[i] = newValue;
        }

        zone[zoneIndex].configureTargets(config.tempTargets, config.humidityTarget);

        display.sendCommand("page zoneDetails");
      }
    }
    else if (command == "SHT")
    {
      int8_t zoneIndex = display.getIntValue(F("global.currentZone"));

      if (zoneIndex >= 0)
      {
        ZoneConfig config = zone[zoneIndex].getConfig();

        int8_t newValue = display.getIntValue("target");

        if (newValue >= 0)
          config.humidityTarget = newValue;

        zone[zoneIndex].configureTargets(config.tempTargets, config.humidityTarget);

        display.sendCommand("page zoneDetails");
      }
    }
    else if (command == "OV")
    {
      isOverview = true;

      for (uint8_t i = 0; i < 4; i++)
      {
        display.sendIndexValue(relayPrefix,i,relayState[i]);
      }
    }
    else if (command == "RZ")
    {
      for (int i = 0; i < 3; i++)
      {
        zone[i].setup(10 + i, uvs[i], heaters[i], rain[i]);
        zone[i].clearHumidityHistory();
        zone[i].clearTempHistory();
        zone[i].clearUVIHistory();
        zone[i].saveToEEPROM();
      }
    }
  }
}

bool timeIsInvalid()
{
  return (hour > 23) || (minute >= 60);
}

void updateTime()
{
  if (hour == nextTimePoll || timeIsInvalid()) {
    hour = display.getIntValue("overview.hour");
    minute = display.getIntValue("overview.minute");
    nextMinute = time + 60000;
    nextTimePoll = hour + timePollInterval;

    if (nextTimePoll >= 24)
      nextTimePoll -= 24;
  }

  if (time > nextMinute) {
    minute++;

    if (minute >= 60)
    {
      minute -= 60;
      hour++;
    }
    
    if (hour >= 24)
      hour -= 24;

    nextMinute = time + 60000;
  }
}

void loop()
{
  time = millis();

  getDisplayCommand();

  updateTime();

  if (time < sampleTime)
    return;

  if (hour < lightOff && hour >= lightOn)
  {
    if (setRelay(lightRelay, HIGH))
      display.sendIndexValue(relayPrefix,lightRelay,HIGH);
  }
  else if (setRelay(lightRelay, LOW))
      display.sendIndexValue(relayPrefix,lightRelay,LOW);

  sampleTime = time + 1000;

  int deltaTime = time - prevTime;
  prevTime = time;

  int refLevel = 1023;//averageAnalogRead(A0);

  for (int i = 0; i < 3; i++)
  {
    zone[i].update(hour, minute, deltaTime, refLevel);

    if (hour != prevHour) {
      zone[i].saveToEEPROM();
      prevHour = hour;
    }

    if (isOverview)
      zone[i].updateDisplayOverview();
  }
}