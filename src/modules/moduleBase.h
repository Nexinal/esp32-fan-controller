#pragma once

#include "fanController.h"
#include "moduleMeta.h"
#include <ArduinoJson.h>


class ModuleBase
{
protected:
    const ModuleMeta meta;
    SettingsCategory &settings;

public:
    ModuleBase(const char *name, const char *version, SettingsManager &settingsManager)
        : meta(ModuleMeta(name, version)),
          settings(*settingsManager.getCategory(name))
    {
    }

    virtual void setup() = 0;
    virtual void loop() = 0;  

    virtual void getInfoForLog(Logger &log) const {
        log.printfln("|> %s Module, version %s", meta.name, meta.version);
    };

    virtual String getInfoForJson() const = 0;

    virtual ModuleMeta getMeta() const
    {
        return meta;
    };

protected:
    JsonDocument startJsonDoc() const
    {
        JsonDocument doc;
        doc["name"] = meta.name;
        doc["version"] = meta.version;
        return doc;
    };
};
