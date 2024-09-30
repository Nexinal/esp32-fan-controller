/*
Copy this file to "config_override.h"
Any defines from "config.h" in CAPITALS can be overridden in "config_override.h".
All defines having BOTH lowercase and uppercase MUST stay in "config.h". They define the mode the "esp32 fan controller" is running in.
If you add additional overrides here, you have to
  1. first add #undef
  2. add new #define
*/
#undef WIFI_SSID
#undef WIFI_PASSWORD
#undef MQTT_SERVER
#undef MQTT_SERVER_PORT
#undef MQTT_USER
#undef MQTT_PASS
#undef UNIQUE_DEVICE_FRIENDLYNAME
#undef UNIQUE_DEVICE_NAME
#undef MQTTCMNDSHUTDOWNTOPIC
#undef MQTTCMNDSHUTDOWNPAYLOAD
#undef TOUCH_CS
#undef TOUCH_IRQ
#undef LED_ON
// --- fan specs ----------------------------------------------------------------------------------------------------------------------------
// fanPWM
#undef PWMPIN
#undef FANMAXRPM
// fanTacho
#undef TACHOPIN




// --- fan specs ----------------------------------------------------------------------------------------------------------------------------
// fanPWM
#define PWMPIN GPIO_NUM_32
#define FANMAXRPM 4500 // only used for showing at how many percent fan is running
#define FANMINRPM 20 // minimum speed at which the fan can run, switch of if below

// fanTacho
#define TACHOPIN GPIO_NUM_34

// Relay / Mosfet
#define RELAY_PIN  GPIO_NUM_25


#ifdef useWIFI
#define WIFI_SSID            "Egzy"         
#define WIFI_PASSWORD        "BakersD0zen"     
#endif

#ifdef useMQTT
#define MQTT_SERVER                "192.168.0.240"
#define MQTT_SERVER_PORT           1883                   
#define MQTT_USER                  "fancontroller"        
#define MQTT_PASS                  "85hJ5YpA1Czs7pYreOYc"
#define MQTT_TOPIC                 "fancontrol"
#define UNIQUE_DEVICE_FRIENDLYNAME "Passive Input Ventilation" 
#define UNIQUE_DEVICE_NAME         "piv_fan" 
#endif

#ifdef useShutdownButton
#define MQTTCMNDSHUTDOWNTOPIC          UNIQUE_DEVICE_NAME "/cmnd/shutdown" 
#define MQTTCMNDSHUTDOWNPAYLOAD        "shutdown"                          
#endif

#ifdef useTFT
#define LED_ON           HIGH          
#endif
#ifdef useTouch
#define TOUCH_CS         GPIO_NUM_14   
#define TOUCH_IRQ        GPIO_NUM_27   
#endif
