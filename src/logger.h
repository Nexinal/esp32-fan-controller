#pragma once
#ifndef LOGGER_H
    #define LOGGER_H

#ifndef ESP8266
    #include <HardwareSerial.h> // ensure we have the correct "Serial" on new MCUs (depends on ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT)
#endif

#ifdef DEBUG_HOST
#include "net_debug.h"
#endif

class Logger
{
private:
#ifdef DEBUG_HOST
    NetworkDebugPrinter NetDebug;
#endif

public:
    Logger();

    void print(const char *message);
    void println(const char *message);
    void print(String message);
    void println(String message);
    void printf(const char *format, ...);
    void printfln(const char *format, ...);

    void printInformation();
    void dumpStats();
    };


    extern Logger Log;
#endif // LOGGER_H