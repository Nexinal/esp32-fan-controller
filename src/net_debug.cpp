#ifdef DEBUG_HOST

#include <WiFi.h>
#include "net_debug.h"
#include "config.h"

size_t NetworkDebugPrinter::write(uint8_t c)
{
    if (!debugPrintHostIP && !debugPrintHostIP.fromString(DEBUG_HOST))
    {
#ifdef ESP8266
        WiFi.hostByName(DEBUG_HOST, debugPrintHostIP, 750);
#else
        WiFi.hostByName(DEBUG_HOST, debugPrintHostIP);
#endif
    }

    debugUdp.beginPacket(debugPrintHostIP, DEBUG_PORT);
    debugUdp.write(c);
    debugUdp.endPacket();
    return 1;
}


size_t NetworkDebugPrinter::write(const uint8_t *buf, size_t size)
{
    if (!debugPrintHostIP && !debugPrintHostIP.fromString(DEBUG_HOST))
    {
#ifdef ESP8266
        WiFi.hostByName(DEBUG_HOST, debugPrintHostIP, 750);
#else
        WiFi.hostByName(DEBUG_HOST, debugPrintHostIP);
#endif
    }

    debugUdp.beginPacket(debugPrintHostIP, DEBUG_PORT);
    size = debugUdp.write(buf, size);
    debugUdp.endPacket();
    return size;
}

#endif

