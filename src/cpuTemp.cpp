#include "fanController.h"
#include "cpuTemp.h"


CPUTemp::CPUTemp(SettingsManager &settingsManager)
    : ModuleBase(CPU_TEMP_MODULE_NAME, CPU_TEMP_MODULE_VERSION, settingsManager) {
        };

CPUTemp::~CPUTemp() {

};

void CPUTemp::setup() {

};

void CPUTemp::loop()
{
#if not defined(ESP8266) && not defined(CONFIG_IDF_TARGET_ESP32S2) // ESP32S2

    // set a default that means we will read the temperature on the first loop
    static unsigned long lastCheck = 0 - INTERNAL_TEMPERATURE_INTERVAL_MS;

    if (millis() - lastCheck < INTERNAL_TEMPERATURE_INTERVAL_MS)
        return;
    lastCheck = millis();

    // temperature, rounded to 2 decimal places
    temperature = roundf(temperatureRead() * 100) / 100;

#ifdef ENABLE_MQTT
    MQTT.publish("cpuTemp", String(temperature));
#endif

    Log.printfln("CPU Temperature: %.2f", temperature);

#endif
}





void CPUTemp::getInfoForLog(Logger &log) const
{
    ModuleBase::getInfoForLog(log);

    log.printfln("|> %s Module, version %s", meta.name, meta.version);
#if defined(ESP8266) || defined(CONFIG_IDF_TARGET_ESP32S2) // ESP32S2
    log.printfln("|> NOT SUPPORTED");
#else
    log.printfln("|> CPU Temperature: %f", temperature);
#endif
};





String CPUTemp::getInfoForJson() const
{
    JsonDocument doc = startJsonDoc();
#if defined(ESP8266) || defined(CONFIG_IDF_TARGET_ESP32S2) // ESP32S2
    doc["temperature"] = "NOT SUPPORTED";

#else
    doc["temperature"] = temperature;
#endif
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

