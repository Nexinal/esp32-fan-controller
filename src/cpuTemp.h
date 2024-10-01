#pragma once
#ifndef CPU_TEMP_H
#define CPU_TEMP_H

#include "config.h"
#include "settings.h"
#include "modules/moduleBase.h"

#define CPU_TEMP_MODULE_NAME "CPU_TEMP"
#define CPU_TEMP_MODULE_VERSION "1.0.0"

class CPUTemp : public ModuleBase
{
    private:
        float temperature = -1;

    public:
        CPUTemp(SettingsManager& settingsManager) ;

        ~CPUTemp();

        void setup() override;
        void loop() override;

        void getInfoForLog(Logger &log) const override;
        String getInfoForJson() const override;

};  // class CPUTemp

#endif // CPU_TEMP_H
