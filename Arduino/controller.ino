#include "DHT.h"
#include "utils.h"
#include "zone.h"
#include "nextionDisplay.h"
#include "relays.h"

DHT dhts[3];

int uvs[] = {
    A3, A2, A1};

int heaters[] = {
    0, -1, 1};

int rain[] = {
    -1, -1, 2};

bool uvEnabled[] = {
    true, true, true};

byte eol[] = {
    0xFF, 0xFF, 0xFF};

long sampleTime = 10000;
long prevTime = 0;

Zone zone1(0, 0);
Zone zone2(150, 1);
Zone zone3(300, 2);

Zone zone[3] = {
    zone1,
    zone2,
    zone3};

bool isOverview = false;
uint8_t updateDisplayCounter = 0;

NextionDisplay display;

uint8_t hour = 0;
uint8_t minute = 0;

long nextMinute = 60000;
uint8_t nextTimePoll = 0;
uint8_t prevHour = 0;

const uint8_t timePollInterval = 6; //6 Hours

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

void loop()
{
  long time = millis();

  if (display.hasCommand())
  {
    String command = display.getCommand();
    if (command == "GetZoneTemps")
    {
      isOverview = false;

      int zoneIndex = display.getIntValue("temps.currentZone");

      if (zoneIndex >= 0)
      {
        ZoneConfig config = zone[zoneIndex].getConfig();

        for (int i = 0; i < 24; i++)
        {
          display.sendIndexValue('h', "", i, config.tempTargets[i]);
        }
      }
    }
    else if (command == "SaveZoneTemps")
    {
      int zoneIndex = display.getIntValue("temps.currentZone");

      if (zoneIndex >= 0)
      {
        ZoneConfig config = zone[zoneIndex].getConfig();

        for (int i = 0; i < 24; i++)
        {
          int newValue = display.getIntValue('h', "", i);

          if (newValue >= 0)
            config.tempTargets[i] = newValue;
        }

        zone[zoneIndex].configureTargets(config.tempTargets, config.humidityTarget);

        display.sendCommand("page overview");
      }
    }
    else if (command == "InitOverview")
    {
      isOverview = true;
    }
  }

  if (hour >= nextTimePoll) {
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

  if (time < sampleTime)
    return;

  sampleTime = time + 1000;

  int deltaTime = time - prevTime;
  prevTime = time;

  updateDisplayCounter++;

  int refLevel = averageAnalogRead(A0);

  for (int i = 0; i < 3; i++)
  {
    zone[i].update(hour, minute, deltaTime, refLevel);

    if (hour != prevHour) {
      zone[i].saveToEEPROM();
      prevHour = hour;
    }

    if (isOverview && updateDisplayCounter > 3)
      zone[i].updateDisplayOverview();
  }

  if (updateDisplayCounter > 3)
    updateDisplayCounter = 0;
}