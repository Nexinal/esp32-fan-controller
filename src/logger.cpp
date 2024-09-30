#include "config.h"
#include "logger.h"
#include <Arduino.h>
#include <esp_system.h>

#undef DEBUG_HOST

#ifndef DISABLE_DEBUG_SERIAL
    #ifdef ESP8266
    #else                       // ESP32
        #include <HardwareSerial.h> // ensure we have the correct "Serial" on new MCUs (depends on ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT)
    #endif
#endif

#ifdef DEBUG_HOST
    #include "net_debug.h"

    // On the host side, use netcat to receive the log statements:  nc -l 7868 -u
    // use -D DEBUG_HOST='"192.168.xxx.xxx"' or FQDN within quotes
    #ifndef DEBUG_PORT
        #define DEBUG_PORT 7868
    #endif
#endif


Logger::Logger()
{
#ifndef DISABLE_DEBUG_SERIAL
    Serial.begin(LOG_SERIAL_BAUD);
#endif

#ifdef DEBUG_HOST
    NetDebug = NetworkDebugPrinter();
#endif
}

void Logger::print(const char *message)
{
#ifndef DISABLE_DEBUG_SERIAL
    Serial.print(message);
#endif
#ifdef DEBUG_HOST
    NetDebug.print(message);
#endif
}

void Logger::println(const char *message)
{
#ifndef DISABLE_DEBUG_SERIAL
    Serial.println(message);
#endif

#ifdef DEBUG_HOST
    NetDebug.println(message);
#endif
}

void Logger::print(String message)
{
    print(message.c_str());
}

void Logger::println(String message) 
{
    println(message.c_str());
}

void Logger::printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[LOG_BUFFER_SIZE];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    print(buffer);
}

void Logger::printfln(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[LOG_BUFFER_SIZE];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    println(buffer);
}


void Logger::printInformation() {
    char buffer[512]; // Adjust size as needed
    int offset = 0;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "|> CPU Info\n"
                       "|> - Chip Model: %s\n"
                       "|> - Chip Revision: %d\n"
                       "|> - CPU Frequency: %d MHz\n"
                       "|> - Number of Cores: %d\n"
                       "|> - SDK Version: %s\n"
                       "|> Flash Info:\n"
                       "|> - Flash Size: %.2f KB\n"
                       "|> - Flash Speed: %lu Hz\n"
                       "|> App Info:\n"
                       "|> - App Size: %lu KB\n"
                       "|> - App MD5: %s\n",
                       ESP.getChipModel(),
                       ESP.getChipRevision(),
                       ESP.getCpuFreqMHz(),
                       ESP.getChipCores(),
                       ESP.getSdkVersion(),
                       ESP.getFlashChipSize() / 1024.0,
                       ESP.getFlashChipSpeed(),
                       ESP.getSketchSize() / 1024,
                       ESP.getSketchMD5().c_str());

    print(buffer);
}

void Logger::dumpStats()
{
    char buffer[512]; // Adjust size as needed
    int offset = 0;

    unsigned long heapSize = ESP.getHeapSize();
    unsigned long freeHeap = ESP.getFreeHeap();

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
        "|> Memory Stats\n"
        "|> - Total Heap: %.2f KB\n"
        "|> - Heap Usage: %.2f KB\n"
        "|> - Free Heap: %.2f KB\n"
        "|> - Min Free Heap: %lu KB\n"
        "|> - Max Alloc Block: %lu KB\n",
        heapSize / 1024.0,
        (heapSize - freeHeap) / 1024.0,
        freeHeap / 1024.0,
        ESP.getMinFreeHeap() / 1024,
        ESP.getMaxAllocHeap() / 1024
    );

    if (psramFound()) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
            "|> - Total PSRAM: %lu KB\n"
            "|> - Free PSRAM: %lu KB\n",
            ESP.getPsramSize() / 1024,
            ESP.getFreePsram() / 1024
        );
    } else {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
            "|> - PSRAM: NONE FOUND\n"
        );
    }

    print(buffer);
}

