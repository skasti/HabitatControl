#ifndef NEXTIONDISPLAY_H
#define NEXTIONDISPLAY_H

#include <inttypes.h>
#include <WString.h>

#if defined(__AVR_ATmega2560__)
#define Nextion Serial3
#else
#define Nextion Serial
#endif

class NextionDisplay {
    private:
        void sendEOL();
        String command;
        char buffer[40];
        int bufPos = 0;
        int eolCounter = 0;
        void extractCommand(int length);
    public:
        void setup();
        void debug(char text[]);
        void sendCommand(char command[]);
        void sendValue(char componentName[], char value[]);
        void sendValue(char componentName[], int value);
        void sendIndexValue(char componentName[], int index, char value[]);
        void sendIndexValue(char componentName[], int index, int value);
        int getIntValue(char componentName[]);
        String getStringValue(char componentName[]);
        bool hasCommand();
        String getCommand();
        int readLine();
};

#endif