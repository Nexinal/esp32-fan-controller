#define DEFINE_GLOBAL_VARS

#include "fanController.h"
#include <vector> // or the appropriate container header
#include "esp_wifi.h"
#include "esp_sleep.h"

#ifdef ENABLE_SLEEP_MODE
void enterSleepMode();
#endif

void setup()
{

    Log.println("");
    Log.println("Starting up");
    Log.println("");

    Log.printInformation();
    Log.println("");

    settingsManager.loadAll();

    // initialise all of the modules
    for (const auto &module : modules)
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

    Network.start();
}


// Never use blocking code in loop() to avoid problems with other tasks
void loop() {
    static unsigned int loopCount = 0;
    static unsigned long totalLoopTimeMillis = 0;
    static unsigned long maxLoopMillis = 0;

    unsigned long now = millis();
    unsigned long thisloopTimeMillis = 0;

    for (const auto &module : modules)
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
        yield();
    }

    // every STATS_INTERVAL milliseconds, dump the stats/diagnostics
    // currently 60 seconds
    static unsigned long lastDebugStats = millis() - (STATS_INTERVAL / 2);
    if (now - lastDebugStats > STATS_INTERVAL)
    {
        lastDebugStats = millis();

        Log.println("|> DEBUG STATS ---------------------------------------------------");
        Log.dumpStats();

        for (const auto &module : modules) {
            module->getInfoForLog(Log);
            yield();

            if (MQTT.getStatus() == MQTT_CONNECTED)
            {
                String json = module->getInfoForJson();
                String topic = "diagnostics/" + String(module->getMeta().name);
                MQTT.publish(topic, json);
                yield();
            }
        }



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
        yield();
    }




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


    // Restart if requested but if settings are dirty
    // do the restart on the next loop to allow time for them
    // to be saved.
    if (restartRequested) {
        if (settingsManager.isDirty()) {
            Log.println("Restart requested but settings need saving.");
        }
        else {

            #ifdef ENABLE_MQTT
            MQTT.publish("STATUS", "offline", true);
            MQTT.disconnect();
            #endif

            Log.println("Rebooting...");
            ESP.restart();
        }
        yield();
    }


    if (factoryResetRequested)
    {
#ifdef ENABLE_MQTT
        MQTT.publish("STATUS", "resetting", true);
        MQTT.disconnect();
#endif

        Log.println("Factory resetting...");

        // clears the settings and restarts the ESP
        settingsManager.factoryReset();
    }


#ifdef ENABLE_SLEEP_MODE
    static unsigned long lastActivity = 0;
    if (thisloopTimeMillis > 2) {
        Log.printf("|>  - Loop took %u ms", thisloopTimeMillis);
        lastActivity = millis();
    }

    if (millis() - lastActivity > IDLE_TIMEOUT_MS)
    {
        enterSleepMode();
    }
#endif

    yield();
}

#ifdef ENABLE_SLEEP_MODE    
void enterSleepMode()
{
    Log.println("Preparing to enter light sleep mode");

    // Configure wake-up sources
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION_uS);

    // Enable wake on WiFi
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    esp_sleep_enable_wifi_wakeup();

        // Wake up on interrupt
    gpio_config_t config = {
        .pin_bit_mask = BIT64(DEFAULT_TACH_PIN),
        .mode = GPIO_MODE_INPUT};
    ESP_ERROR_CHECK(gpio_config(&config));

    gpio_wakeup_enable(DEFAULT_TACH_PIN,  GPIO_INTR_LOW_LEVEL);

    esp_sleep_enable_gpio_wakeup();

    // Enter light sleep mode
    esp_light_sleep_start();

    // Code continues here after waking up
    Log.println("Woke up from light sleep");

    // Check wake-up reason
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_WIFI)
    {
        Log.println("Woke up due to WiFi activity");
    }
    else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
    {
        Log.println("Woke up due to timer");
    }

    // Disable wake on WiFi after waking up
    esp_sleep_disable_wifi_wakeup();
}
#endif