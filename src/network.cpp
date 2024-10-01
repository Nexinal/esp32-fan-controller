#include "fanController.h"
#include "network.h"

#define WIFI_CHECK_DELAY_MS 500

NetworkController::NetworkController(SettingsManager &settingsManager)
    : ModuleBase(WIFI_MODULE_NAME, WIFI_MODULE_VERSION, settingsManager)
{
    settings.addSetting("hostname", new Setting<String>(DEFAULT_DEVICE_NAME, 15));
    settings.addSetting("ssid", new Setting<String>(DEFAULT_WIFI_SSID, 32));
    settings.addSetting("password", new Setting<String>(DEFAULT_WIFI_PASSWORD, 63));
}


NetworkController::~NetworkController()
{
}

void NetworkController::setup()
{
#ifdef ESP8266
    WiFi.hostname(DEFAULT_DEVICE_NAME);
#else
    WiFi.setHostname(settings.getValue<String>("hostname").c_str());
#endif

    // Add WiFi event listener
    WiFi.onEvent(std::bind(&NetworkController::WiFiEvent, this, std::placeholders::_1));

   
    // Calculate the maximum number of loops based on the timeout and delay
    // WIFI_TIMEOUT is seconds, WIFI_CHECK_DELAY_MS is milliseconds
    wifiMaxLoops = (WIFI_TIMEOUT * 1000UL) / WIFI_CHECK_DELAY_MS;

    //wifiReconnectLoopCount = wifiMaxLoops * 2 / 3;
}


void NetworkController::loop()
{
    if (mode == NetworkMode::ACCESS_POINT) {
        // check for connected clients and provide captive portal
    }
}


void NetworkController::start() {
    if (connect() != NetworkStatus::CONNECTED)
    {
        startAcessPoint();
    }
}

NetworkStatus NetworkController::connect()
{
    if (mode == NetworkMode::ACCESS_POINT) {
        WiFi.softAPdisconnect();
    }

    mode = NetworkMode::STATION;
    String ssid = settings.getValue<String>("ssid");

    if (ssid == "")
    {
        setLastMessage("No WiFi SSID configured");
        status = NetworkStatus::FAILED;
        return NetworkStatus::FAILED;
    }

    Log.printfln("Connecting to WiFi - SSID: %s", ssid.c_str());
    Log.printfln("MAC: %s", WiFi.macAddress().c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, settings.getValue<String>("password"));

    int wifiLoopCount = 0;
    while (WiFi.status() != WL_CONNECTED && wifiLoopCount < wifiMaxLoops)
    {
        Log.print(".");
        //Log.printfln("%d : %d", wifiLoopCount, static_cast<int>(WiFi.status()));

        if (WiFi.status() == WL_NO_SSID_AVAIL)
        {
            setLastMessage("WiFi SSID not available");
            break;
        }

        delay(WIFI_CHECK_DELAY_MS);
        wifiLoopCount += 1;

        // // Try a reconnect when 2/3rds of loops are complete
        // if (wifiLoopCount == wifiReconnectLoopCount)
        // {
        //     WiFi.reconnect();
        // }
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        status = NetworkStatus::FAILED;
        setLastMessage("WiFi connect failed: " + String(WiFi.status()));
    } else {
        status = NetworkStatus::CONNECTED;
        Log.println("Wifi connected successfully");
    }

    return status;
}


void NetworkController::startAcessPoint()
{
#ifdef ENABLE_AP_MODE
    Log.println("Starting Access Point");
    mode = NetworkMode::ACCESS_POINT;

    WiFi.softAPConfig(IPAddress(4, 3, 2, 1), IPAddress(4, 3, 2, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_DEFAULT_SSID, AP_DEFAULT_PASS, 1, false);
#else
    NetworkStatusstatus = NetworkStatus::FAILED;
    Log.println("WiFi AP mode not enabled. Reboot to try again.");
#endif
}




void NetworkController::WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {

        //
        // Station Events
        //

        case SYSTEM_EVENT_WIFI_READY:
            setLastMessage("WiFi Ready");
            status = NetworkStatus::DISCONNECTED;
            break;

        case SYSTEM_EVENT_SCAN_DONE:
            setLastMessage("WiFi Scan Done");
            break;

        case SYSTEM_EVENT_STA_START:
            status = NetworkStatus::CONNECTING;
            mode = NetworkMode::STATION;
            setLastMessage("WiFi Station Start");
            break;

        case SYSTEM_EVENT_STA_STOP:
            status = NetworkStatus::DISCONNECTED;
            mode = NetworkMode::OFF;
            setLastMessage("WiFi Station Stop");
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            status = NetworkStatus::CONNECTED;
            setLastMessage("WiFi Connected to SSID: " + WiFi.SSID());
            Log.printfln("WiFi strength: %d%% (%d dBm)", getSignalQuality(WiFi.RSSI()), WiFi.RSSI());
            Log.printfln("MAC: %s", WiFi.macAddress().c_str());
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            status = NetworkStatus::DISCONNECTED;
            setLastMessage("WiFi Disconnected");
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            status = NetworkStatus::CONNECTED;
            setLastMessage("IP: " + WiFi.localIP().toString());
            break;

        case SYSTEM_EVENT_GOT_IP6:
            setLastMessage("WiFi Got IP6");
            break;

        case SYSTEM_EVENT_STA_LOST_IP:
            status = NetworkStatus::CONNECTING;
            setLastMessage("WiFi Lost IP");
            break;

        case SYSTEM_EVENT_STA_BSS_RSSI_LOW:
            setLastMessage("WiFi BSS RSSI Low");
            Log.printfln("WiFi strength: %d dBm %d%%", WiFi.RSSI(), getSignalQuality(WiFi.RSSI()));
            break;

        //
        // Access Point Events
        //

        case SYSTEM_EVENT_AP_START:
            mode = NetworkMode::ACCESS_POINT;
            setLastMessage("WiFi AP Start");
            break;

        case SYSTEM_EVENT_AP_STOP:
            mode = NetworkMode::OFF;
            setLastMessage("WiFi AP Stop");
            break;

        case SYSTEM_EVENT_AP_STACONNECTED:
            setLastMessage("WiFi AP Station Connected");
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
            setLastMessage("WiFi AP Station Disconnected");
            break;

        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            setLastMessage("WiFi AP Station IP Assigned");
            break;

        case SYSTEM_EVENT_AP_PROBEREQRECVED:
            setLastMessage("WiFi AP Probe Request Received");
            break;
    }
}

void NetworkController::getInfoForLog(Logger &log) const
{
    ModuleBase::getInfoForLog(log);

    std::pair<int, String> modeInfo = getFriendlyMode();
    log.printfln("|>  - Mode: %d (%s)", modeInfo.first, modeInfo.second.c_str());

    std::pair<int, String> statusInfo = getFriendlyStatus();
    log.printfln("|>  - Status: %d (%s)", statusInfo.first, statusInfo.second.c_str());

    log.printfln("|>  - Last: %s", lastMessage.c_str());

    if (mode == NetworkMode::OFF) {
        log.println("|>  - WiFi is OFF");
    }

    if (mode == NetworkMode::STATION) {
        log.printfln("|>  - Signal: %d dBm %d%%", WiFi.RSSI(), getSignalQuality(WiFi.RSSI()));
        log.printfln("|>  - IP: %s", WiFi.localIP().toString().c_str());
        log.printfln("|>  - MAC: %s", WiFi.macAddress().c_str());
        log.printfln("|>  - Hostname: %s", WiFi.getHostname());
        log.printfln("|>  - Channel: %d", WiFi.channel());
        log.printfln("|>  - SSID: %s", WiFi.SSID().c_str());
        log.printfln("|>  - BSSID: %s", WiFi.BSSIDstr().c_str());
        log.printfln("|>  - Gateway: %s", WiFi.gatewayIP().toString().c_str());
        log.printfln("|>  - Subnet: %s", WiFi.subnetMask().toString().c_str());
        log.printfln("|>  - DNS1: %s", WiFi.dnsIP().toString().c_str());
        log.printfln("|>  - DNS2: %s", WiFi.dnsIP(1).toString().c_str());
    }

    if (mode == NetworkMode::ACCESS_POINT) {
        log.printfln("|>  - SoftAP IP: %s", WiFi.softAPIP().toString());
        log.printfln("|>  - SoftAP MAC: %s", WiFi.softAPmacAddress().c_str());
        log.printfln("|>  - SoftAP SSID: %s", WiFi.softAPSSID().c_str());
    }
}

String NetworkController::getInfoForJson() const
{
    JsonDocument doc = startJsonDoc();

    std::pair<int, String> modeInfo = getFriendlyMode();
    doc["mode"] = modeInfo.second;

    std::pair<int, String> statusInfo = getFriendlyStatus();
    doc["status"] = statusInfo.second;

    doc["lastMessage"] = lastMessage;

    if (mode == NetworkMode::OFF)
    {
        doc["state"] = "OFF";
    }

    if (mode == NetworkMode::STATION)
    {
        doc["state"] = "STATION";
        doc["signal"]["rssi"] = WiFi.RSSI();
        doc["signal"]["quality"] = getSignalQuality(WiFi.RSSI());
        doc["ip"] = WiFi.localIP().toString();
        doc["mac"] = WiFi.macAddress();
        doc["hostname"] = WiFi.getHostname();
        doc["channel"] = WiFi.channel();
        doc["ssid"] = WiFi.SSID();
        doc["bssid"] = WiFi.BSSIDstr();
        doc["gateway"] = WiFi.gatewayIP().toString();
        doc["subnet"] = WiFi.subnetMask().toString();
        doc["dns1"] = WiFi.dnsIP().toString();
        doc["dns2"] = WiFi.dnsIP(1).toString();
    }

    if (mode == NetworkMode::ACCESS_POINT)
    {
        doc["state"] = "ACCESS_POINT";
        doc["softap"]["ip"] = WiFi.softAPIP().toString();
        doc["softap"]["mac"] = WiFi.softAPmacAddress();
        doc["softap"]["ssid"] = WiFi.softAPSSID();
    }

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}


std::pair<int, String> NetworkController::getFriendlyMode() const
{
    int modeID = WiFi.getMode();
    String modeString;
    switch (modeID)
    {
    case WIFI_MODE_NULL:
        modeString = "NULL";
        break;
    case WIFI_MODE_STA:
        modeString = "STA";
        break;
    case WIFI_MODE_AP:
        modeString = "AP";
        break;
    case WIFI_MODE_APSTA:
        modeString = "APSTA";
        break;
    default:
        modeString = "UNKNOWN";
        break;
    }

    return std::make_pair(modeID, modeString);
}

std::pair<int, String> NetworkController::getFriendlyStatus() const
{
    int statusID = WiFi.status();
    String statusString;
    switch (statusID)
    {
    case WL_IDLE_STATUS:
        statusString = "WL_IDLE_STATUS";
        break;
    case WL_NO_SSID_AVAIL:
        statusString = "WL_NO_SSID_AVAIL";
        break;
    case WL_SCAN_COMPLETED: 
        statusString = "WL_SCAN_COMPLETED";
        break;
    case WL_CONNECTED:
        statusString = "WL_CONNECTED";
        break;
    case WL_CONNECT_FAILED:
        statusString = "WL_CONNECT_FAILED";
        break;      
    case WL_CONNECTION_LOST:
        statusString = "WL_CONNECTION_LOST";
        break;
    default:
        statusString = "UNKNOWN";
        break;
    }

    return std::make_pair(statusID, statusString);
    
}

void NetworkController::reconnect()
{
    WiFi.reconnect();
}


bool NetworkController::isConnected() const
{
    return status == NetworkStatus::CONNECTED;
}

wl_status_t NetworkController::getStatus() const
{
    return WiFi.status();
};


// by https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp
int NetworkController::getSignalQuality(int rssi) const
{
    int quality = 0;

    if (rssi <= -100)
    {
        quality = 0;
    }
    else if (rssi >= -50)
    {
        quality = 100;
    }
    else
    {
        quality = 2 * (rssi + 100);
    }
    return quality;
}

void NetworkController::setLastMessage(String message)
{
    Log.println("WIFI: " + message);
    lastMessage = message;
}

String NetworkController::getLastMessage() const
{
    return lastMessage;
}

