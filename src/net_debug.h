#pragma once

#ifdef DEBUG_HOST

#include <cstddef>
#include <Print.h>
#include <IPAddress.h>
#include <WiFiUdp.h>

class NetworkDebugPrinter : public Print
{
private:
    WiFiUDP debugUdp;
    IPAddress debugPrintHostIP;

public:
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buf, size_t size) override;
};

#endif // DEBUG_HOST