#include "fanController.h"
#include "fanPWM.h"

#define FAN_LOOP_INTERVAL_MILLIS 200    // 5 times a second
#define FAN_REPORT_TO_MQTT_INTERVAL_MILLIS 5000  // every 5 second


FanPWM::FanPWM(SettingsManager& settingsManager)
    : ModuleBase(FAN_PWM_MODULE_NAME, FAN_PWM_MODULE_VERSION, settingsManager)
{
    settings.addSetting("startSpeed", new Setting<short>(DEFAULT_POWER_ON_SPEED));
    settings.addSetting("fanPin", new Setting<byte>(DEFAULT_PWM_PIN));
    settings.addSetting("relayPin", new Setting<byte>(DEFAULT_RELAY_PIN));

    settings.addSetting("maxRPM", new Setting<int>(DEFAULT_MAX_RPM));
    settings.addSetting("minPercent", new Setting<byte>(DEFAULT_MIN_PERCENT));
    settings.addSetting("minStartPercent", new Setting<byte>(MIN_START_PERCENT));

    settings.addSetting("pmwFrequency", new Setting<int>(PWM_FREQ));
    settings.addSetting("pmwChannel", new Setting<byte>(PWM_CHANNEL));
    settings.addSetting("pmwResolution", new Setting<byte>(PWM_RESOLUTION));

    #ifdef ENABLE_MQTT
    MQTT.registerCallback("fan", std::bind(&FanPWM::handleCommands, this, std::placeholders::_1, std::placeholders::_2));
    #endif
}

FanPWM::~FanPWM()
{
}

void FanPWM::setup()
{
    short startSpeed = settings.getValue<short>("startSpeed");
    byte fanpin = settings.getValue<byte>("fanPin");
    byte relayPin = settings.getValue<byte>("relayPin");

    int pmwFrequency = settings.getValue<int>("pmwFrequency");
    byte pmwChannel = settings.getValue<byte>("pmwChannel");
    byte pmwResolution = settings.getValue<byte>("pmwResolution");

    // setup relay pin, start with it off
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);

    // configure LED PWM functionalitites
    ledcSetup(pmwChannel, pmwFrequency, pmwResolution);

    // attach the channel to the GPIO to be controlled
    ledcAttachPin(fanpin, pmwChannel);

    setSpeed(startSpeed);
}


void FanPWM::loop()
{
    //only every 100ms
    static unsigned long chaseTargetSpeedMillis = 0;
    if (millis() - chaseTargetSpeedMillis > FAN_LOOP_INTERVAL_MILLIS)
    {
        chaseTargetSpeedMillis = millis();

        // If targetSpeedPercent is not the same as CurrentSpeedPercent
        // then we need to move Current towards the target but dio it over
        // a number of loop
        if (targetSpeedPercent != currentSpeedPercent)
        {
            if (currentSpeedPercent < targetSpeedPercent)
            {
                currentSpeedPercent++;
            }
            else
            {
                currentSpeedPercent--;
            }

            int pwmValue = getPWMValue(currentSpeedPercent);
            ledcWrite(PWM_CHANNEL, pwmValue);
        }
    }

#ifdef ENABLE_MQTT
    static unsigned long reportToMqttMillis = 0;
    if (millis() - reportToMqttMillis > FAN_REPORT_TO_MQTT_INTERVAL_MILLIS)
    {
        reportToMqttMillis = millis();
        reportToMQTT();
    }
#endif
}


void FanPWM::setSpeed(int requestedSpeedPercent)
{
    byte relayPin = settings.getValue<byte>("relayPin");

    if (requestedSpeedPercent == 0 && isRunning)
    {
        isRunning = false;
        currentSpeedPercent = 0;
        digitalWrite(relayPin, LOW);

#ifdef ENABLE_MQTT
        reportToMQTT();
#endif
        return;
    }

    uint8_t minPercent = settings.getValue<byte>("minPercent");

    targetSpeedPercent = min(requestedSpeedPercent, 100);
    if (targetSpeedPercent < minPercent)
    {
        targetSpeedPercent = minPercent;
    }

    if (isRunning) {
        currentSpeedPercent = targetSpeedPercent;
    }
    else {
        // Not running, lets start it up
        isRunning = true;
        digitalWrite(relayPin, HIGH);

        uint8_t minStartPercent = settings.getValue<byte>("minStartPercent");

        if (targetSpeedPercent < minStartPercent)
        {
            currentSpeedPercent = minStartPercent;
        }
        else {
            currentSpeedPercent = targetSpeedPercent;
        }
    }


    Log.printfln("FANPWM:setSpeed - target %u%%, current %u%%", targetSpeedPercent, currentSpeedPercent);

#ifdef ENABLE_MQTT
    reportToMQTT();
#endif  

    int pwmValue = getPWMValue(currentSpeedPercent);

    Log.printfln("FANPWM:setSpeed - Setting fan speed to %u%% (%u)", currentSpeedPercent, pwmValue);
    ledcWrite(PWM_CHANNEL, pwmValue);
}


void FanPWM::getInfoForLog(Logger &log) const
{
    ModuleBase::getInfoForLog(log);

    byte relayPin = settings.getValue<byte>("relayPin");
    byte fanPin = settings.getValue<byte>("fanPin");    
    log.printfln("Relay Pin: %u", relayPin);
    log.printfln("Relay GPIO: %s", digitalRead(relayPin) ? "HIGH" : "LOW");
    log.printfln("Fan Pin: %u", fanPin);
    log.printfln("Current Speed: %u%%", currentSpeedPercent);
    log.printfln("Target Speed: %u%%", targetSpeedPercent);
    log.printfln("Is Running: %s", isRunning ? "Yes" : "No");
    log.printfln("PWM Value: %u", String(getPWMValue(currentSpeedPercent)));
    log.printfln("PWM Resolution: %u", String(settings.getValue<byte>("pmwResolution")));
    log.printfln("PWM Frequency: %u", String(settings.getValue<int>("pmwFrequency")));
    log.printfln("PWM Channel: %u", String(settings.getValue<byte>("pmwChannel")));


}

String FanPWM::getInfoForJson() const
{
    JsonDocument doc = startJsonDoc();

    byte relayPin = settings.getValue<byte>("relayPin");
    byte fanPin = settings.getValue<byte>("fanPin");
    
    doc["relayPin"] = relayPin;
    doc["relayGPIO"] = digitalRead(relayPin) ? "HIGH" : "LOW";
    doc["fanPin"] = fanPin;
    doc["currentSpeed"] = currentSpeedPercent;
    doc["targetSpeed"] = targetSpeedPercent;
    doc["isRunning"] = isRunning ? "Yes" : "No";
    doc["pwmValue"] = String(getPWMValue(currentSpeedPercent));
    doc["pwmResolution"] = String(settings.getValue<byte>("pmwResolution"));    
    doc["pwmFrequency"] = String(settings.getValue<int>("pmwFrequency"));
    doc["pwmChannel"] = String(settings.getValue<byte>("pmwChannel"));

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

#ifdef ENABLE_MQTT
void FanPWM::reportToMQTT()
{
    if (MQTT.getStatus() == MQTT_CONNECTED)
    {
        MQTT.publish("currentSpeed", String(currentSpeedPercent));
        MQTT.publish("targetSpeed", String(targetSpeedPercent));
        MQTT.publish("isRunning", String(isRunning));
    }
}
#endif

void FanPWM::handleCommands(const String& command, const String& payload)
{
    Log.printfln("FANPWM:handleCommands - command %s, payload %s", command, payload);

    if (command == "setSpeed")
    {
        setSpeed(payload.toInt());
    }
}

int FanPWM::getPWMValue(int speedPercent) const
{
    byte pmwResolution = settings.getValue<byte>("pmwResolution");
    int maxValue = (1 << pmwResolution) - 1;
    int pwmValue = (speedPercent * maxValue) / 100;
    return pwmValue;
}
