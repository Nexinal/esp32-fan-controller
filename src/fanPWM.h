#pragma once
#ifndef FAN_PWM_H
#define FAN_PWM_H

#define FAN_PWM_MODULE_NAME "FanPWM"
#define FAN_PWM_MODULE_VERSION "1.0"

#include <Arduino.h>
#include <esp32-hal.h>
#include <esp32-hal-ledc.h>

#include "config.h"
#include "settings.h"
#include "modules/moduleBase.h"

class FanPWM : public ModuleBase
{
    private:
        byte currentSpeedPercent = 0;
        byte targetSpeedPercent = 0;
        bool isRunning = false;


    public:
        FanPWM(SettingsManager& settingsManager);
        ~FanPWM();
        
        void setup() override;
        void loop() override;

        void getInfoForLog(Logger &log) const override;
        String getInfoForJson() const override;

        void setSpeed(int requestedSpeedPercent);

#ifdef ENABLE_MQTT
        void reportToMQTT();
#endif

    private:
        void handleCommands(const String& command, const String& payload);
        int getPWMValue(int speedPercent) const;
};  
#endif