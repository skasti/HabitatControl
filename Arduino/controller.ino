#include "DHT.h"
#include "utils.h"

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

long sampleTime = 0;
long resetTime = 0;
float uvSeconds[] = {
  0.0, 0.0, 0.0
};

byte eol[] = {
  0xFF, 0xFF, 0xFF
};

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
  Serial.begin(9600);

  pinMode(A0, INPUT);
  
  for (int i = 0; i < 3; i++) {
    pinMode(uvs[i], INPUT);
    //digitalWrite(uvs[i], HIGH);

    // if (analogRead(uvs[i]) >= 1000)
    //   uvEnabled[i] = false;
    // else
    //   digitalWrite(uvs[i], LOW);
  }

  for (int i = 0; i < 3; i++) {
    dhts[i].setup(10 + i);
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

  sampleTime = time + 1000;

  if (time > resetTime) {
    resetTime = millis() + 120000;

    uvSeconds[0] = 0;
    uvSeconds[1] = 0;
    uvSeconds[2] = 0;
  }

  int refLevel = averageAnalogRead(A0);

  for (int i = 0; i < 3; i++) {
    float humidity = dhts[i].getHumidity();
    float temperature = dhts[i].getTemperature();

    if (uvEnabled[i]) {
      //Use the 5V power pin as a reference to get a very accurate output value from sensor
      int uvLevel = averageAnalogRead(uvs[i]);
      float outputVoltage = (5.0 / refLevel) * uvLevel;
      float uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level

      if ((uvIntensity < 15) && (uvIntensity >= 1))
        uvSeconds[i] += uvIntensity;

      sendUVI(i, (int)(uvIntensity));
    }

    sendUVs(i, uvSeconds[i]);
    sendTemp(i, (int)(temperature));
    sendHumidity(i, (int)(humidity));    

    if (uvSeconds[i] > 30.0 && relayState[i] == HIGH) {
      setRelay(i, LOW);
    } else if (uvSeconds[i] <= 30.0 && relayState[i] == LOW) {
      setRelay(i, HIGH);
    }
  }
}