#include "nextionDisplay.h"
#include "utils.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


void NextionDisplay::setup() 
{
    Nextion.begin(9600);
}

void NextionDisplay::sendEOL()
{
    Nextion.write(0xFF);
    Nextion.write(0xFF);
    Nextion.write(0xFF);
}

void NextionDisplay::sendCommand(char command[])
{
    Nextion.print(command);
    sendEOL();
}

void NextionDisplay::sendValue(char componentName[], char value[])
{
    Nextion.print(componentName);
    Nextion.print(".txt=\"");
    Nextion.print(value);
    Nextion.print("\"");
    sendEOL();
}

void NextionDisplay::sendValue(char componentName[], int value)
{
    Nextion.print(componentName);
    Nextion.print(".val=");
    Nextion.print(value);
    sendEOL();
}

void NextionDisplay::sendWaveFormValue(int objId, int channel, int value)
{
    Nextion.print("add ");
    Nextion.print(objId);
    Nextion.print(",");
    Nextion.print(channel);
    Nextion.print(",");
    Nextion.print(value);
    sendEOL();
}

void NextionDisplay::sendIndexValue(char prefix, char componentName[], int index, char value[])
{
    Nextion.print(prefix);
    Nextion.print(index);
    Nextion.print(componentName);
    Nextion.print(".txt=");
    Nextion.print(value);
    sendEOL();
}

void NextionDisplay::sendIndexValue(char prefix, char componentName[], int index, int value)
{
    Nextion.print(prefix);
    Nextion.print(index);
    Nextion.print(componentName);
    Nextion.print(".val=");
    Nextion.print(value);
    sendEOL();
}

void NextionDisplay::debug(char text[])
{
    sendValue("debug", text);
}

void NextionDisplay::extractCommand(int endIndex)
{
    char commandBuffer[endIndex + 1];

    for(int i = 1; i <= endIndex; i++)
        commandBuffer[i- 1] = buffer[i];

    commandBuffer[endIndex] = 0;

    command = String(commandBuffer);
}

bool NextionDisplay::hasCommand()
{
    int endIndex = readLine();
    if ((endIndex > 0) && (buffer[0] == 'p'))
    {
        extractCommand(endIndex);
        return true;
    }

    return false;
}

String NextionDisplay::getCommand()
{
    return command;
}

int NextionDisplay::getIntValue(char componentName[])
{
    //Empty inbound buffer, as we dont want any other data than what we request
    while(Nextion.available())
    {
        Nextion.read();
    }

    Nextion.print("get ");
    Nextion.print(componentName);
    Nextion.print(".val");
    sendEOL();

    int attempts = 0;

    while (attempts < 500) {
        if (readLine() == 4) {
            ArrayToInteger converter;
            
            for (int i = 0; i < 4; i++)
            {
                converter.array[i] = buffer[1+i];
            }

            return converter.integer;
            //return (int)buffer[1];
        }
        delay(2);
        attempts++;
    }

    return -1;
}

int NextionDisplay::getIntValue(char prefix, char componentName[], int index)
{
    //Empty inbound buffer, as we dont want any other data than what we request
    while(Nextion.available())
    {
        Nextion.read();
    }

    Nextion.print("get ");
    Nextion.print(prefix);
    Nextion.print(index);
    Nextion.print(componentName);
    Nextion.print(".val");
    sendEOL();

    int attempts = 0;

    while (attempts < 500) {
        if (readLine() == 4) {
            ArrayToInteger converter;
            
            for (int i = 0; i < 4; i++)
            {
                converter.array[i] = buffer[1+i];
            }

            return converter.integer;
            // return (int)buffer[1];
        }
        delay(2);
        attempts++;
    }

    return -1;
}

String NextionDisplay::getStringValue(char componentName[])
{
    //Empty inbound buffer, as we dont want any other data than what we request
    while(Nextion.available())
    {
        Nextion.read();
    }

    Nextion.print("get ");
    Nextion.print(componentName);
    Nextion.print(".txt");
    sendEOL();

    int attempts = 0;

    while (attempts < 500) {
        if (hasCommand()) {
            return getCommand();
        }
        delay(1);
        attempts++;
    }

    return "ERR";
}


String NextionDisplay::getStringValue(char prefix, char componentName[], int index)
{
    //Empty inbound buffer, as we dont want any other data than what we request
    while(Nextion.available())
    {
        Nextion.read();
    }

    Nextion.print("get ");
    Nextion.print(prefix);
    Nextion.print(index);
    Nextion.print(componentName);
    Nextion.print(".txt");
    sendEOL();

    int attempts = 0;

    while (attempts < 500) {
        if (hasCommand()) {
            return getCommand();
        }
        delay(1);
        attempts++;
    }

    return "ERR";
}

int NextionDisplay::readLine()
{
    while(Nextion.available())
    {
        byte c = Nextion.read();

        if ((buffer[0] != 'p') && (buffer[0] != 'q'))
            bufPos = 0;

        buffer[bufPos] = c;

        if (c == 0xFF)
            eolCounter++;
        else
            eolCounter = 0;

        if (eolCounter == 3)
        {
            int endIndex = bufPos-3;
            bufPos = 0;
            eolCounter = 0;
            return endIndex;
        }

        bufPos++;

        if (bufPos >= 40)
        {
            debug("HICK");
            bufPos = 0;
            eolCounter = 0;
            return -1;
        }
    }

    return 0;
}