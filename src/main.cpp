#define DEFINE_GLOBAL_VARS

#include "fanController.h"
#include <vector> // or the appropriate container header

void setup()
{
    Log.println("");
    Log.println("Starting up");
    Log.println("");

    Log.printInformation();
    Log.println("");

    settingsManager.loadAll();

    // initialise all of the modules
    for (auto module : modules)
    {
        // monitor the time the module takes to setup and log an message if more than WATCHDOG_MAX_SETUP_MILLIS
        unsigned long setupStart = millis();
        module->setup();
        unsigned long setupTime = millis() - setupStart;
        if (setupTime > WATCHDOG_MAX_SETUP_MILLIS)
        {
            Log.printfln("Module %s took %u ms to setup", module->getMeta().name, setupTime);
        }
    }


    // initialise settings repository
    //settings.load();

    // initialise module repository

    // load all modules
    // connect to wifi
    // connect to mqtt

    Log.dumpStats();
}


// Never use blocking code in loop() to avoid problems with other tasks
void loop() {
    static unsigned int loopCount = 0;
    static unsigned long totalLoopTimeMillis = 0;
    static unsigned long maxLoopMillis = 0;

    unsigned long now = millis();
    unsigned long thisloopTimeMillis = 0;

    for (const auto& module : modules)
    {
        //Log.printfln("Module %s loop", module->getMeta().name);

        // monitor the time the module takes to loop and log an message if more than WATCHDOG_MAX_LOOP_MILLIS
        unsigned long loopStart = millis();
        module->loop();
        unsigned long loopTime = millis() - loopStart;
        if (loopTime > WATCHDOG_MAX_LOOP_MILLIS)
        {
            Log.printfln("Module %s took %u ms to loop", module->getMeta().name, loopTime);
        }

        yield;
    }


    // every 500ms, check if settings are dirty and save if they are
    static unsigned long settingsCheckMillis = 0;
    if (now - settingsCheckMillis > 500)
    {
        if (settingsManager.isDirty()) {
            Log.println("** Settings are dirty, saving");
            settingsManager.saveAll();
        }
        settingsCheckMillis = millis();
    }



    // every 10 seconds, call Log.dumpStats().
    static unsigned long lastDebugStats = 0;
    if (now - lastDebugStats > STATS_INTERVAL)
    {
        lastDebugStats = millis();

        Log.println("|> DEBUG STATS ---------------------------------------------------");
        Log.dumpStats();
        Network.getInfoForLog(Log);

        // avoid division by zero
        if (loopCount == 0)
            loopCount = 1;

        int statsSeconds = STATS_INTERVAL / 1000;
        unsigned int loopsPerSecond = loopCount / statsSeconds;
        
        Log.println("|> Loop Information");
        Log.printfln("|>  - Loop Count : %d", loopCount);
        Log.printfln("|>  - Loop/second: %u", loopsPerSecond);
        Log.printfln("|>  - Total Loop Time: %u ms", totalLoopTimeMillis);
        Log.printfln("|>  - Average Loop Time: %.2f ms", static_cast<float>(totalLoopTimeMillis) / loopCount);
        Log.printfln("|>  - Maximum Loop Time: %u ms", maxLoopMillis);

        loopCount = 0;
        maxLoopMillis = 0;
        totalLoopTimeMillis = 0;

        Log.println("|> ---------------------------------------------------------------");
    }
    yield();





    // Loop housekeeping and monitoring

    thisloopTimeMillis = millis() - now;
    if (thisloopTimeMillis > maxLoopMillis) {
        maxLoopMillis = thisloopTimeMillis;
    }
    totalLoopTimeMillis += thisloopTimeMillis;
    loopCount++;

    if (thisloopTimeMillis > WATCHDOG_SLOW_LOOP_TIME)
    {
        Log.println ("**************************************************");
        Log.println ("*>  Slow Loop");
        Log.printfln("*>  - Took %u ms", thisloopTimeMillis);
        Log.printfln("*>  - Avg Time: %u ms", totalLoopTimeMillis / loopCount);
        Log.printfln("*>  - Loop Count: %d", loopCount);
        Log.printfln("*>  - Max Loop: %u ms", maxLoopMillis);
        Log.println ("**************************************************");
    }

    yield();
}