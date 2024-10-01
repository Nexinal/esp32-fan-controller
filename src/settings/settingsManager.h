#pragma once
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <string>
#include <map>
#include "SettingBase.h"
#include "SettingsCategory.h"
#include "../fanController.h"
#include "../logger.h"
#include <nvs_flash.h>
#define EEPROM_SIZE 512
#define SETTINGS_HEADER_BYTE 0xFA

class SettingsManager
{
private:
    bool isInitialized = false;
    std::map<std::string, SettingsCategory> categories;

public:
    SettingsManager()  {

        if (nvs_flash_init() != ESP_OK) {
            Log.println("ERROR! NVS flash initialization failed");
        }
        
        else if (!EEPROM.begin(EEPROM_SIZE))
        {
            Log.println("ERROR! EEPROM initialization failed");
        }
    }

    SettingsCategory &addCategory(const std::string &name)
    {
        SettingsCategory& newCategory = categories[name];
        return newCategory;
    }

    SettingsCategory *getCategory(const std::string &name)
    {
        // Find name in categories, if not found, create a new one and return it
        auto it = categories.find(name);
        if (it == categories.end())
        {
            if (isInitialized)
            {
                Log.printfln("SettingsManager.getCategory: Category %s not found", name.c_str());
                return nullptr;
            }
            return &addCategory(name);
        }
        return &it->second;
    }
    

    void saveAll() {
        if (!isDirty()) {return;}

        JsonDocument doc; 
        JsonObject root = doc.to<JsonObject>();

        for (auto &pair : categories)
        {
            JsonObject categoryObj = root[pair.first].to<JsonObject>();
            pair.second.toJson(categoryObj);
        }

        String jsonString;
        serializeJson(doc, jsonString);

        // Log.println("SettingsManager.saveAll: ");
        // Log.printfln("String length: %d", jsonString.length() + 1);
        // Log.println(jsonString.c_str());

        // Write a header to the EEPROM to indicate the start of the settings
        EEPROM.writeByte(0, SETTINGS_HEADER_BYTE);
        EEPROM.writeString(1, jsonString);

        // // Write the JSON string to the EEPROM
        // for (size_t i = 0; i < jsonString.length(); i++)
        // {
        //     EEPROM.write(i + 1, jsonString[i]);
        // }
        // EEPROM.write(jsonString.length() + 1, 0); // Null terminator

        // Commit the changes to EEPROM
        if (EEPROM.commit())
        {
            Log.println("EEPROM successfully committed");
        }
        else
        {
            Log.println("ERROR! EEPROM commit failed");
        }

        // Verify contents of EEPROM
        Log.println("Verifying EEPROM contents: ");
        Log.printf("Header: %d, ", EEPROM.read(0));
        for (int i = 1; i <= 25 && i <= jsonString.length(); i++)
        {
            Log.printf("%c", EEPROM.read(i));
        }
        Log.println("");
    }

    void loadAll() {

        isInitialized = true;

        // Read the header from the EEPROM to check if the settings are valid
        byte header = EEPROM.readByte(0);
        if (header != SETTINGS_HEADER_BYTE)
        {
            Log.printfln("Settings: Invalid header: %d", header );
            return;
        }

        String jsonString = EEPROM.readString(1);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonString);

        if (error == DeserializationError::EmptyInput) {
            Log.println("Settings: No settings found");
            return;
        }

        if (error) {
            Log.print("Settings: Error deserializing JSON");
            Log.println(error.c_str());
            return;
        }

        JsonObject root = doc.as<JsonObject>();
        for (JsonPair pair : root)
        {
            const char* categoryName = pair.key().c_str();
            JsonObject categoryObj = pair.value().as<JsonObject>();
            categories[categoryName].fromJson(categoryObj);
        }
    }


    // clear the settings and restart the ESP
    void factoryReset() 
    {
        // Clear the EEPROM
        EEPROM.writeByte(0, 0);
        EEPROM.commit();

        // restart the ESP
        ESP.restart();
    }

    bool isDirty() const
    {       
        for (const auto& [name, category] : categories) {
            if (category.isDirty()) {
                return true;
            }
        }
        return false;
    }

    size_t totalSize() const
    {
        size_t total = 0;
        for (auto &pair : categories)
        {
            total += pair.second.size();
        }
        return total;
    }
};