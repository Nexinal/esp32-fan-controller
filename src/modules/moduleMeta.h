#pragma once

#include <Arduino.h>

struct ModuleMeta
{
    char name[32];
    char version[12];  

    // at creation, check the name and version for memory overflow
    ModuleMeta(const char *moduleName, const char *moduleVersion)
    {
        strncpy(name, moduleName, sizeof(name));
        strncpy(version, moduleVersion, sizeof(version));
    }
};
