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
        void extractCommand(int endIndex);
        char componentNameBuffer[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    public:
        void setup();
        void debug(char text[]);
        void sendCommand(char command[]);
        void sendValue(char componentName[], char value[]);
        void sendValue(char componentName[], int value);
        void sendWaveFormValue(int objId, int channel, int value);
        void sendIndexValue(char prefix, char componentName[], int index, char value[]);
        void sendIndexValue(char prefix, char componentName[], int index, int value);
        void sendIndexValue(char prefix, int index, char value[]);
        void sendIndexValue(char prefix, int index, int value);
        int getIntValue(const __FlashStringHelper* componentName);
        int getIntValue(char componentName[]);
        String getStringValue(char componentName[]);
        int getIntValue(char prefix, char componentName[], int index);
        String getStringValue(char prefix, char componentName[], int index);
        int getIntValue(char prefix, int index);
        String getStringValue(char prefix, int index);        
        bool hasCommand();
        String getCommand();
        int readLine();
};

#endif