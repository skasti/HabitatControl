#include "DHT.h"
#include "utils.h"
#include "zone.h"
#include "nextionDisplay.h"

DHT dhts[3];

int uvs[] = {
  A3, A2, A1
};

int heaters[] = {
  0, -1, 1
};


int rain[] = {
  -1, -1, 2
};

bool uvEnabled[] = {
  true, true, true
};

int numRelays = 4;
int relays[] = {
  2,3,4,5
};

int relayState[] = {
  LOW, LOW, LOW, LOW
};

byte eol[] = {
  0xFF, 0xFF, 0xFF
};

long sampleTime = 10000;
long prevTime = 0;

Zone zone1(0,0);
Zone zone2(150,1);
Zone zone3(300,2);

Zone zone[3] = {
  zone1,
  zone2,
  zone3
};

bool isOverview = false;

NextionDisplay display;

void setup() {
  display.setup();

  pinMode(A0, INPUT);
  
  for (int i = 0; i < 3; i++) {
    //if (!zone[i].loadFromEEPROM()) {
      zone[i].setup(10+i, uvs[i], heaters[i],rain[i]);
    //}


  }

  for (int i = 0; i < numRelays; i++) {
    pinMode(relays[i], OUTPUT);
    digitalWrite(relays[i], relayState[i]);
  }   
}

void setRelay(int relay, int state) {

  if (relayState[relay] == state)
    return;

  relayState[relay] = state;

  digitalWrite(relays[relay], state);

  //sendRelayState(relay);
}

void loop() {
  long time = millis();

  if (display.hasCommand()) 
  {
    String command = display.getCommand();
    if (command == "GetZoneTemps")
    {
      int zoneIndex = display.getIntValue("temps.currentZone");

      if (zoneIndex >= 0)
      {
        isOverview = false;
        ZoneConfig config = zone[zoneIndex].getConfig();

        for (int i = 0; i < 24; i++)
        {
          display.sendIndexValue('h', "",i, config.tempTargets[i]);
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
          int newValue = display.getIntValue('h', "",i);

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

  if (time < sampleTime)
    return;

  sampleTime = time + 1000;

  int deltaTime = time - prevTime;
  prevTime = time;

  int refLevel = averageAnalogRead(A0);

  for (int i = 0; i < 3; i++) {
    zone[i].update(0,0, deltaTime, refLevel);

    if (isOverview)
      zone[i].updateDisplayOverview();
  }
}