#include "DHT.h"
#include "utils.h"
#include "zone.h"
#include "nextionDisplay.h"

DHT dhts[3];

int uvs[] = {
  A3, A2, A1
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

NextionDisplay display;

void submit() {
  Serial.write(eol, 3);
}

void sendValue(String objectId, int value) {
  Serial.print(objectId);
  Serial.print(".val=");
  Serial.print(value);
  submit();
}

void sendTemp(int index, int value) {
  Serial.print("s");
  Serial.print(index);
  Serial.print("Temp.val=");
  Serial.print(value);
  submit();
}

void sendHumidity(int index, int value) {
  Serial.print("s");
  Serial.print(index);
  Serial.print("Humidity.val=");
  Serial.print(value);
  submit();
}

void sendUVI(int index, int value) {
  Serial.print("s");
  Serial.print(index);
  Serial.print("UV.val=");
  Serial.print(value);
  submit();
}

void sendUVs(int index, int value) {
  Serial.print("s");
  Serial.print(index);
  Serial.print("UVs.val=");
  Serial.print(value);
  submit();
}

void sendRelayState(int index) {
  Serial.print("r");
  Serial.print(index);

  if (relayState[index] == LOW)
    Serial.print(".val=0");
  else
    Serial.print(".val=1");

  submit();
}

void setup() {
  display.setup();

  pinMode(A0, INPUT);
  
  for (int i = 0; i < 3; i++) {
    if (!zone[i].loadFromEEPROM()) {
      zone[i].setup(10+i, uvs[i], i,-1);
    }


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

  sendRelayState(relay);
}

void loop() {
  long time = millis();

  if (time < sampleTime)
    return;

  sampleTime = time + 5000;

  int deltaTime = time - prevTime;
  prevTime = time;

  int refLevel = averageAnalogRead(A0);

  for (int i = 0; i < 2; i++) {
    zone[i].update(0,0, deltaTime, refLevel);
    //zone[i].updateDisplayOverview();
    sendUVI(i, zone[i].getUVI());    
    sendUVs(i, zone[i].getUVIS());
    sendTemp(i, zone[i].getTemp());
    sendHumidity(i, zone[i].getHumidity());
  }
}