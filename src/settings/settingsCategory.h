#pragma once
#include <ArduinoJson.h>
#include <string>
#include <map>
#include "SettingBase.h"
#include "logger.h"

class SettingsCategory
{
private:
    std::map<std::string, SettingBase*> settings;

public:
    template <typename T>
    void addSetting(const std::string &name, Setting<T> *setting)
    {
        settings[name] = setting;
    }

    template <typename T>
    T getValue(const std::string &name) const
    {
        auto it = settings.find(name);
        if (it != settings.end()) {
            if (const Setting<T>* typedSetting = static_cast<const Setting<T>*>(it->second)) {
                return typedSetting->getValue();
            } else {
                throw std::runtime_error("Type mismatch for setting: " + name);
            }
        } else {
            throw std::out_of_range("Setting not found: " + name);
        }
    }

    // Add this overload for string literals
    void setValue(const std::string &name, const char* value)
    {
        setValue<String>(name, String(value));
    }

    template <typename T>
    void setValue(const std::string &name, const T &value)
    {
        auto it = settings.find(name);
        if (it != settings.end()) {
            if (Setting<T>* typedSetting = static_cast<Setting<T>*>(it->second)) {
                typedSetting->setValue(value);
            } else {
                throw std::runtime_error("Type mismatch for setting: " + name);
            }
        } else {
            Log.printfln("Setting not found: %s ", name.c_str());
            Log.println("use AddSetting() first or check the category name is correct");
        }
    }

    

    void toJson(JsonObject &json) const
    {
        for (const auto &pair : settings)
        {
            pair.second->toJson(json, pair.first);
        }
    }

    void fromJson(const JsonObject &json)
    {
        for (const auto &pair : settings)
        {
            pair.second->fromJson(json, pair.first);
        }
    }

    bool isDirty() const
    {
        for (const auto &pair : settings)
        {
            if (pair.second->isDirty())
                return true;
        }
        return false;
    }

    size_t size() const
    {
        size_t totalSize = 0;
        for (const auto& pair : settings)
        {
            totalSize += pair.second->size();
        }
        return totalSize;
    }
};