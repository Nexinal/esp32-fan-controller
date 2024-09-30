#pragma once
#include "SettingBase.h"
#include <ArduinoJson.h>
#include <cstring>

template <typename T>
class Setting : public SettingBase
{
private:
    T value;
    size_t maxLength; // size of the data type, or max length of the string

public:
    Setting(T defaultValue)
        : SettingBase(), value(defaultValue), maxLength(sizeof(T)) {}

    Setting(T defaultValue, size_t maxLen )
        : SettingBase(), value(defaultValue), maxLength(maxLen) {}

    void setValue(T newValue)
    {
        if (value != newValue) {
            if constexpr (std::is_same<T, String>::value)
            {
                value = newValue.substring(0, maxLength);
            }
            else
            {
                value = newValue;
            }
            dirty = true;
        }
    }



    T getValue() const { return value; }



    std::string getValueAsString() const
    {
        if constexpr (std::is_same<T, String>::value || std::is_same<T, std::string>::value)
        {
            return std::string(value.c_str());
        }
        else if constexpr (std::is_arithmetic<T>::value)
        {
            return std::to_string(value);
        }
        else
        {
            // For other types, you might need to implement a custom conversion
            // or use a more advanced technique like std::stringstream
            return "Unsupported type";
        }
    }



    void toJson(JsonObject& json, const std::string& name) override
    {
        json[name] = value;
        dirty = false;

        //Log.printfln("toJson %s: %s, dirty=%d", name.c_str(), value.c_str(), dirty);
    };

    void fromJson(const JsonObject& json, const std::string& name) override
    {
        if (json[name].is<T>()) {
            setValue(json[name].as<T>());
            dirty = false;

            //Log.printfln("fromJson %s: %s, dirty=%d", name.c_str(), value.c_str(), dirty);
        }
    }

    size_t size() const override
    {
        if constexpr (std::is_same<T, String>::value)
        {
            return maxLength + 1; // +1 for null terminator
        }
        else
        {
            return sizeof(T);
        }
    }
};