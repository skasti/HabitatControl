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

void NextionDisplay::sendIndexValue(char componentName[], int index, char value[])
{
    char buf[sizeof(componentName)];
    sprintf(buf, componentName, index);

    sendValue(buf, value);
}

void NextionDisplay::sendIndexValue(char componentName[], int index, int value)
{
    char buf[sizeof(componentName)+1];
    sprintf(buf, componentName, index);

    sendValue(buf, value);
}

void NextionDisplay::debug(char text[])
{
    sendValue("debug", text);
}

void NextionDisplay::extractCommand(int length)
{
    char commandBuffer[length + 1];

    for(int i = 1; i < length+2; i++)
        commandBuffer[i- 1] = buffer[i];

    command = String(commandBuffer);
}

bool NextionDisplay::hasCommand()
{
    if ((readLine() > 1) && (buffer[0] == 'p'))
    {
        extractCommand(bufPos-3);
        
        bufPos = 0;
        eolCounter = 0;
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
        if (readLine() == 5) {
            ArrayToInteger converter;
            
            for (int i = 0; i < 4; i++)
            {
                converter.array[i] = buffer[1+i];
            }

            return converter.integer;
        }

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

        attempts++;
    }

    return "ERR";
}

int NextionDisplay::readLine()
{
    while(Nextion.available())
    {
        byte c = Nextion.read();
        buffer[bufPos] = c;

        if (c == 0xFF)
        {
            eolCounter++;

            if (eolCounter >= 3)
            {
                buffer[bufPos - 0] = 0x00;
                buffer[bufPos - 1] = 0x00;
                buffer[bufPos - 2] = 0x00;

                return bufPos-3;
            }
        }
        else
        {
            eolCounter = 0;
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