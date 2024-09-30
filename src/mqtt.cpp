#include "fanController.h" 

#ifdef ENABLE_MQTT



MQTTController::MQTTController(SettingsManager &settingsManager)
    : ModuleBase(MQTT_MODULE_NAME, MQTT_MODULE_VERSION, settingsManager),
    mqttClient(wifiClient),
    wifiConnected(false)
{
    settings.addSetting("server", new Setting<String>(MQTT_SERVER, 63));
    settings.addSetting("port", new Setting<int>(MQTT_SERVER_PORT));
    settings.addSetting("username", new Setting<String>(MQTT_USER, 32));
    settings.addSetting("password", new Setting<String>(MQTT_PASS, 32));
    settings.addSetting("topic", new Setting<String>(MQTT_TOPIC, 32));
}

MQTTController::~MQTTController()
{
}

void MQTTController::setup()
{
    // Add WiFi event listener
    WiFi.onEvent(std::bind(&MQTTController::WiFiEvent, this, std::placeholders::_1));

}

void MQTTController::loop()
{
}

void MQTTController::WiFiEvent(WiFiEvent_t event)
{
    switch (event) {
        case SYSTEM_EVENT_STA_LOST_IP
        case SYSTEM_EVENT_STA_DISCONNECTED:
            wifiConnected = false;
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
        case SYSTEM_EVENT_GOT_IP6:
            wifiConnected = true;
            break;
    }
}

void MQTTController::publish(const String& topic, const String& payload)
{
    Log.println("MQTTController::publish()");
}

void MQTTController::subscribe(const String& topic)
{
    Log.println("MQTTController::subscribe()");
}

void MQTTController::onMessage(const String& topic, const String& payload)
{
    Log.println("MQTTController::onMessage()");
}

void MQTTController::getInfoForLog(Logger &log) const
{
    log.printfln("|> %s Module, version %s", meta.name, meta.version);
};

String MQTTController::getInfoForJson() const
{
    return String("MQTT");
}

#endif // ENABLE_MQTT
