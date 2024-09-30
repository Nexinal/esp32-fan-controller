#pragma once
#include "config.h"

#ifdef ENABLE_MQTT

#ifndef MQTT_H
#define MQTT_H

#define MQTT_MODULE_NAME "MQTT"
#define MQTT_MODULE_VERSION "1.0"

#include "settings.h"
#include "modules/moduleBase.h"

#include <PubSubClient.h>


class MQTTController : public ModuleBase
{
    private:
        bool wifiConnected;
        PubSubClient mqttClient;

    public:
        MQTTController(SettingsManager& settingsManager);
        ~MQTTController();

        void setup() override;
        void loop() override;

        void getInfoForLog(Logger &log) const override;
        String getInfoForJson() const override;

        void publish(const String& topic, const String& payload);
        void subscribe(const String& topic);

    private:
        void WiFiEvent(WiFiEvent_t event);
        void onMessage(const String& topic, const String& payload);
};








#endif // MQTT_H

#endif // ENABLE_MQTT