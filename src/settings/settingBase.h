#pragma once
#include <ArduinoJson.h>
#include <string>

class SettingBase
{
    protected:
        bool dirty = false;
public:
    virtual void toJson(JsonObject &json, const std::string &name) = 0;
    virtual void fromJson(const JsonObject &json, const std::string &name) = 0;
    virtual size_t size() const = 0;
    virtual std::string getValueAsString() const = 0;


    SettingBase() : dirty(true) {}

    bool isDirty() const {
        return dirty;
    };
};