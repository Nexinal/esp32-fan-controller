#ifndef WIFI_H
#define WIFI_H

#define WIFI_MODULE_NAME "WiFi"
#define WIFI_MODULE_VERSION "1.0"

#include "config.h"
#include "settings.h"
#include "modules/moduleBase.h"

#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266mDNS.h>
    #include <ESPAsyncTCP.h>
#else // ESP32
    #include <WiFi.h>
#endif

enum NetworkStatus {
    DISCONNECTED,
    CONNECTED,
    CONNECTING,
    FAILED
};

enum NetworkMode {
    OFF,
    STATION,
    ACCESS_POINT
};

class NetworkController : public ModuleBase
{
    private:
        NetworkStatus status = NetworkStatus::DISCONNECTED;
        NetworkMode mode = NetworkMode::OFF;
        String lastMessage = "";

        // connection variables
        short wifiMaxLoops = 0;
        short wifiLoopCount = 0;
        //short wifiReconnectLoopCount = 0;
        

    public:
        NetworkController(SettingsManager& settingsManager);
        ~NetworkController();

        void setup() override;
        void loop() override;
        
        NetworkStatus connect();
        void reconnect();

        void startAcessPoint();

        bool isConnected() const;
        wl_status_t getStatus() const;
        String getLastMessage() const;

        void getInfoForLog(Logger &log) const override;
        String getInfoForJson() const override;


    private:
        void WiFiEvent(WiFiEvent_t event);
        int getSignalQuality(int rssi) const;
        void setLastMessage(String message);
        std::pair<int, String> getFriendlyMode() const;
        std::pair<int, String> getFriendlyStatus() const;
};


#endif // WIFI_H
