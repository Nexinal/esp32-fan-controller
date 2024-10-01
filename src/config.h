#pragma once

#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef USE_MY_CONFIG
  #include "my_config.h"
#endif

#ifndef DEFAULT_DEVICE_FRIENDLYNAME
  #define DEFAULT_DEVICE_FRIENDLYNAME "Fan Controller"
#endif

// version code in format yymmddb (b = daily build)
#ifndef VERSION
  #define VERSION 2409270 // 2409270 = 24.09.27.0
#endif




// ----------------------------------------------------------------
// FAN Defaults
// ----------------------------------------------------------------
#ifndef DEFAULT_POWER_ON_SPEED
#define DEFAULT_POWER_ON_SPEED 20
#endif

#ifndef DEFAULT_PWM_PIN
  #define DEFAULT_PWM_PIN GPIO_NUM_32
#endif

#ifndef DEFAULT_MAX_RPM
  #define DEFAULT_MAX_RPM 4500 // only used for showing at how many percent fan is running
#endif

#ifndef DEFAULT_MIN_PERCENT
  #define DEFAULT_MIN_PERCENT 20 // minimum speed at which the fan can run, switch of if below
#endif

#ifndef MIN_START_PERCENT
  #define MIN_START_PERCENT 30 // minimum speed at which the fan can run, switch of if below
#endif

#ifndef PWM_FREQ
  #define PWM_FREQ 25000
#endif

#ifndef PWM_CHANNEL
  #define PWM_CHANNEL 0
#endif

#ifndef PWM_RESOLUTION
  #define PWM_RESOLUTION 8
#endif


// Relay / Mosfet
#ifndef DEFAULT_RELAY_PIN
  #define DEFAULT_RELAY_PIN GPIO_NUM_25
#endif




// fanTacho
#ifndef DEFAULT_TACH_PIN
  #define DEFAULT_TACH_PIN GPIO_NUM_34
#endif

#define TACHO_UPDATE_CYCLE 1000       // how often tacho speed shall be determined, in milliseconds
#define NUMB_INTERRUPS_PER_ROTATION 2 // Number of interrupts ESP32 sees on tacho signal on a single fan rotation. All the fans I've seen trigger two interrups.




// ----------------------------------------------------------------
// Internal temperature sensor
// ----------------------------------------------------------------
#ifndef USE_INTERNAL_TEMPERATURE_SENSOR
  #define USE_INTERNAL_TEMPERATURE_SENSOR
#endif  

#ifndef INTERNAL_TEMPERATURE_INTERVAL_MS
  #define INTERNAL_TEMPERATURE_INTERVAL_MS 5000 // 5 seconds
#endif



// ----------------------------------------------------------------
// Logging Defaults
// ----------------------------------------------------------------
#ifndef LOG_SERIAL_BAUD
  #define LOG_SERIAL_BAUD 115200
#endif

#ifndef LOG_BUFFER_SIZE
  #define LOG_BUFFER_SIZE 512
#endif

#ifndef DEBUG_HOST
  #define DEBUG_HOST "192.168.0.240"
#endif

#ifndef DEBUG_PORT
  #define DEBUG_PORT 7868
#endif

#ifndef STATS_INTERVAL
  #define STATS_INTERVAL 60000  // 60 seconds
#endif




// ----------------------------------------------------------------
// WIFI Defaults
// ----------------------------------------------------------------
#ifndef DEFAULT_WIFI_SSID
  #define DEFAULT_WIFI_SSID ""
#endif

#ifndef DEFAULT_WIFI_PASSWORD
  #define DEFAULT_WIFI_PASSWORD ""
#endif

#ifndef DEFAULT_DEVICE_NAME
  #define DEFAULT_DEVICE_NAME "fan_controller"
#endif

#ifndef WIFI_TIMEOUT
#define WIFI_TIMEOUT 10 // seconds
#endif



// ----------------------------------------------------------------
// WIFI Access Point Defaults
// ----------------------------------------------------------------

#ifndef DISABLE_AP_MODE
  #define ENABLE_AP_MODE
#endif

#ifdef ENABLE_AP_MODE
  #ifndef AP_DEFAULT_SSID
    #define AP_DEFAULT_SSID "Fan_AP"
  #endif

  #ifndef AP_DEFAULT_PASS
    #define AP_DEFAULT_PASS "fan_setup$123"
  #endif
#endif





// ----------------------------------------------------------------
// MQTT Defaults
// ----------------------------------------------------------------

#ifndef DISABLE_MQTT
  #define ENABLE_MQTT 
#endif

#ifdef ENABLE_MQTT
  #ifndef MQTT_SERVER
    #error MQTT_SERVER is not defined
  #endif

  #ifndef MQTT_SERVER_PORT
    #define MQTT_SERVER_PORT 1883
  #endif

  #ifndef MQTT_USER
    #define MQTT_USER ""
  #endif

  #ifndef MQTT_PASS
    #define MQTT_PASS ""
  #endif

  #ifndef MQTT_TOPIC
    #define MQTT_TOPIC "airflow"
  #endif

#ifndef MQTT_CONNECTION_TIMEOUT_SECS
#define MQTT_CONNECTION_TIMEOUT_SECS 10 // seconds
#endif

#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE 1280
#endif

#ifndef MQTT_KEEP_ALIVE
#define MQTT_KEEP_ALIVE 60
#endif

#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 10
#endif
#endif


// ----------------------------------------------------------------
// Over-The-Air Updates
// ----------------------------------------------------------------
// #define DISABLE_OTA         // saves 14kb




// ----------------------------------------------------------------
// WebSockets / HTTP Server
// ----------------------------------------------------------------
#ifndef DISABLE_WEBSOCKETS
  #define ENABLE_WEBSOCKETS
#endif



// ----------------------------------------------------------------
// Watchdog and Monitoring
// ----------------------------------------------------------------
#ifndef WATCHDOG_SLOW_LOOP_TIME
#define WATCHDOG_SLOW_LOOP_TIME 50 // milliseconds
#endif

#ifndef WATCHDOG_MAX_SETUP_MILLIS
#define WATCHDOG_MAX_SETUP_MILLIS 5
#endif

#ifndef WATCHDOG_MAX_LOOP_MILLIS
#define WATCHDOG_MAX_LOOP_MILLIS 5
#endif

// ----------------------------------------------------------------
// Power management and sleep mode
// This is currently experimental and shouldn't be used. 
// ----------------------------------------------------------------
//#define ENABLE_SLEEP_MODE

#ifdef ENABLE_SLEEP_MODE
  #define IDLE_TIMEOUT_MS 1000      // 1 second of inactivity before sleeping
  #define SLEEP_DURATION_uS 1000000 // Sleep for 1 second (in microseconds)
#endif

#endif /*__CONFIG_H__*/



