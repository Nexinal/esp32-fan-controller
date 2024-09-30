#pragma once
#ifndef FANCONTROLLER_H
#define FANCONTROLLER_H


#include <wstring.h>
#include <vector>
#include <Arduino.h>

#include "config.h"
#include "logger.h"
#include "settings.h"

#include "./modules/module.h"
#include "network.h"
#include "mqtt.h"

#ifndef DISABLE_OTA
    #define NO_OTA_PORT
    #include <ArduinoOTA.h>
#endif





// ----------------------------------------------------------------
// Global Variables
// ----------------------------------------------------------------

#ifndef DEFINE_GLOBAL_VARS
    #define GLOBAL extern
    #define _INIT(x)
    #define _INIT_N(x)
#else
    #define GLOBAL
    #define _INIT(x) = x

    // needed to ignore commas in array definitions
    #define UNPACK(...) __VA_ARGS__
    #define _INIT_N(x) UNPACK x
#endif

#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)




GLOBAL char versionString[] _INIT(TOSTRING(VERSION));

GLOBAL Logger Log _INIT(Logger());
GLOBAL SettingsManager settingsManager _INIT(SettingsManager());


// Modules
GLOBAL NetworkController Network _INIT(NetworkController(settingsManager));


#ifdef ENABLE_MQTT
GLOBAL MQTTController MQTT _INIT(MQTTController(settingsManager));
//modules.add(MQTT);
#endif

GLOBAL std::vector<ModuleBase *> modules _INIT_N(({&Network, &MQTT}));

#endif  // FANCONTROLLER_H

