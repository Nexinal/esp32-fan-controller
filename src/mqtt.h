#pragma once
#include "config.h"

#ifdef ENABLE_MQTT

#ifndef MQTT_H
#define MQTT_H

#define MQTT_MODULE_NAME "MQTT"
#define MQTT_MODULE_VERSION "1.0"
#define MAX_CALLBACKS 10

#include "settings.h"
#include "modules/moduleBase.h"
#include <WiFiClient.h>
#include <PubSubClient.h>


class MQTTController : public ModuleBase
{
    private:
        int mqttMaxLoops = 0;
        String clientName;
        bool mqttConnectionDesired;
        bool wifiConnected;
        PubSubClient mqttClient;
        String lastMessage = "";
        WiFiClient wifiClient;
        String topic = MQTT_TOPIC;

        struct CallbackEntry
        {
            String moduleName;
            std::function<void(const String &, const String &)> callback;
        };
        
        static CallbackEntry moduleCallbacks[MAX_CALLBACKS];
        static int callbackCount;

    public:
        MQTTController(SettingsManager& settingsManager);
        ~MQTTController();

        void setup() override;
        void loop() override;

        int connect();
        void disconnect();

        void getInfoForLog(Logger &log) const override;
        String getInfoForJson() const override;

        void publish(const String &topic, const String &payload);
        void publish(const String &topic, const String &payload, bool retained);

        void publish(const String& topic, const String& payload, const String& rootTopic, bool retained);

        void subscribe(const String &topic);
        void subscribe(const String &topic, int qos);
        void subscribe(const String &topic, String &rootTopic,int qos);

        void unsubscribe(const String &topic);
        void unsubscribe(const String &topic, String &rootTopic);

        int getStatus() const;
        String getLastMessage() const;

        static void registerCallback(const String& moduleName, std::function<void(const String&, const String&)> callback);

    private:
        int establishConnection();
        void WiFiEvent(WiFiEvent_t event);
        void onMessage(const String& topic, const String& payload);
        void setLastMessage(String message);

        const char * getClientName()  const;

        std::pair<int, String> getFriendlyStatus() const;
};








#endif // MQTT_H

#endif // ENABLE_MQTT