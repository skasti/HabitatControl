#ifndef RELAYS_H
#define RELAYS_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

const int numRelays = 4;
const int relays[] = {
    2, 3, 4, 5};

static int relayState[] = {
    LOW, LOW, LOW, LOW};

static bool setRelay(int relay, int state)
{
    if (relayState[relay] == state)
        return false;

    relayState[relay] = state;
    digitalWrite(relays[relay], state);

    return true;
}

static void setupRelays()
{
    for (int i = 0; i < numRelays; i++)
    {
        pinMode(relays[i], OUTPUT);
        digitalWrite(relays[i], relayState[i]);
    }
}

#endif