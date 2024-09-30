#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <map>
#include <string>
    
#include "settings/SettingBase.h"
#include "settings/Setting.h"
#include "settings/SettingsCategory.h"
#include "settings/SettingsManager.h"

extern SettingsManager settingsManager;