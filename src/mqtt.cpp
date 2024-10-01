#include "fanController.h" 

#ifdef ENABLE_MQTT

#define MQTT_CHECK_DELAY_MS 100


    MQTTController::MQTTController(SettingsManager &settingsManager)
        : ModuleBase(MQTT_MODULE_NAME, MQTT_MODULE_VERSION, settingsManager),
        mqttClient( PubSubClient(wifiClient) ),
        mqttConnectionDesired(true),
        wifiConnected(false)
    {
        settings.addSetting("server", new Setting<String>(MQTT_SERVER, 15));
        settings.addSetting("port", new Setting<int>(MQTT_SERVER_PORT));
        settings.addSetting("username", new Setting<String>(MQTT_USER, 32));
        settings.addSetting("password", new Setting<String>(MQTT_PASS, 32));
        settings.addSetting("topic", new Setting<String>(MQTT_TOPIC, 32));
    }

    MQTTController::~MQTTController()
    {
    }

    void MQTTController::setup()
    {
        // Calculate the maximum number of loops based on the timeout and delay
        // MQTT_CONNECTION_TIMEOUT is seconds, MQTT_CHECK_DELAY_MS is milliseconds
        mqttMaxLoops = (MQTT_CONNECTION_TIMEOUT_SECS * 1000UL) / MQTT_CHECK_DELAY_MS;

        mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
        mqttClient.setKeepAlive(MQTT_KEEP_ALIVE);
        mqttClient.setSocketTimeout(MQTT_SOCKET_TIMEOUT);

        mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
            String topicStr = String(topic);
            String payloadStr = String((char*)payload, length);
            this->onMessage(topicStr, payloadStr);
        });

        // Add WiFi event listener
        WiFi.onEvent(std::bind(&MQTTController::WiFiEvent, this, std::placeholders::_1));
    }

    int MQTTController::connect() {
        mqttConnectionDesired = true;
        return establishConnection();
    }

    void MQTTController::disconnect()
    {
        mqttConnectionDesired = false;
        mqttClient.disconnect();
    }

    int MQTTController::establishConnection()
    {
        if (!wifiConnected || !mqttConnectionDesired)
        {
            return MQTT_DISCONNECTED;
        }

        if (mqttClient.state() == MQTT_CONNECTED)
        {
            return MQTT_CONNECTED;
        }


        // set topic from the settings, suffixed with the client name
        topic = settings.getValue<String>("topic") + "/" + getClientName();
        
        // create a last will topic by prefixing the topic with "stat/status"
        String lastWillTopic = topic + "/STATUS";

        String server = settings.getValue<String>("server");
        const int port = settings.getValue<int>("port");
        String username = settings.getValue<String>("username");
        String password = settings.getValue<String>("password");

        Log.printfln("Connecting to MQTT server: %s:%d", server, port);
        mqttClient.setServer(server.c_str(), port);

        mqttClient.connect(getClientName(),
                    username.c_str(), password.c_str(),
                    lastWillTopic.c_str(), 
                    1 /* qos */, 
                    true /* retained */, 
                    "offline", 
                    false /* clear session */);

        int loopCount = 0;
        while (mqttClient.state() != MQTT_CONNECTED && loopCount < mqttMaxLoops)
        {
            Log.print(".");
            delay(MQTT_CHECK_DELAY_MS);
            loopCount++;

            if (mqttClient.state() == MQTT_CONNECT_BAD_CREDENTIALS 
                || mqttClient.state() == MQTT_CONNECT_UNAUTHORIZED
                || mqttClient.state() == MQTT_CONNECT_BAD_CLIENT_ID
                || mqttClient.state() == MQTT_CONNECT_UNAVAILABLE
                || mqttClient.state() == MQTT_CONNECT_BAD_PROTOCOL
                || mqttClient.state() == MQTT_CONNECT_FAILED
                || mqttClient.state() == MQTT_CONNECTION_LOST
                || mqttClient.state() == MQTT_CONNECTION_TIMEOUT
                )
            {
                break;
            }
        }

        if (mqttClient.state() != MQTT_CONNECTED)
        {
            setLastMessage("MQTT connect failed: " + getFriendlyStatus().second);
        }
        else
        {
            setLastMessage("MQTT connected successfully");
            publish("STATUS", "online", true);
            subscribe("COMMAND/#");
        }

        return mqttClient.state();
    }

    void MQTTController::loop()
    {
        mqttClient.loop();
    }

    void MQTTController::WiFiEvent(WiFiEvent_t event)
    {
        switch (event) {
            case SYSTEM_EVENT_STA_LOST_IP:
            case SYSTEM_EVENT_STA_DISCONNECTED:
                setLastMessage("WiFi disconnected");
                wifiConnected = false;
                break;

            case SYSTEM_EVENT_STA_GOT_IP:
            case SYSTEM_EVENT_GOT_IP6:
                setLastMessage("WiFi connected");
                wifiConnected = true;
                establishConnection();
            break;
        }
    }



    void MQTTController::publish(const String& topic, const String& payload)
    {
        publish(topic, payload, this->topic, false);
    }

    void MQTTController::publish(const String &topic, const String &payload, bool retained) {
        publish(topic, payload, this->topic, retained);
    }


    void MQTTController::publish(const String &topic, const String &payload, const String &rootTopic, bool retained)
    {
        String fullTopic = rootTopic + "/" + topic;
        mqttClient.publish(fullTopic.c_str(), payload.c_str(), retained);
    }




    void MQTTController::subscribe(const String& topic)
    {
        subscribe(topic, this->topic, 0);
    }

    void MQTTController::subscribe(const String &topic, int qos)
    {
        subscribe(topic, this->topic, qos);
    }

    void MQTTController::subscribe(const String &topic, String &rootTopic, int qos)
    {
        String fullTopic = rootTopic + "/" + topic;
        mqttClient.subscribe(fullTopic.c_str(), qos);
    }



    void MQTTController::unsubscribe(const String &topic)
    {
        unsubscribe(topic, this->topic);
    }

    void MQTTController::unsubscribe(const String &topic, String &rootTopic)
    {
        String fullTopic = rootTopic + "/" + topic;
        mqttClient.unsubscribe(fullTopic.c_str());
    }



    void MQTTController::onMessage(const String& topic, const String& payload)
    {
        // Get the actual command by removing the this->topic prefix and the the word "COMMAND"
        // airflow/piv_fan/COMMAND/mode becomes "mode"
        // airflow/piv_fan/COMMAND/fan/speed becomes "fan/speed"
        String command = topic.substring(this->topic.length() + 9);

        if (command.length() == 0) {
            setLastMessage("MQTT message arrived with empty command");
            return;
        }

        Log.printfln("MQTT message arrived [%s] %s", command.c_str(), payload.c_str());

        // Split the command into the module and the command
        int slashIndex = command.indexOf('/');
        String module = "system";
        if (slashIndex != -1) {
            module = command.substring(0, slashIndex);
            command = command.substring(slashIndex + 1);
        }

        // Check if there's a registered callback for this module
        auto callback = moduleCallbacks.find(module);
        if (callback != moduleCallbacks.end()) {
            // Execute the callback
            callback->second(command, payload);
        } else {
            Log.printfln("No callback registered for module: %s", module.c_str());
        }
    }



    void MQTTController::getInfoForLog(Logger &log) const
    {
        ModuleBase::getInfoForLog(log);

        Log.printfln("|>  - Client Name: %s", getClientName());
        Log.printfln("|>  - Desired: %d", mqttConnectionDesired);
        log.printfln("|>  - Last: %s", lastMessage.c_str());

        std::pair<int, String> status = getFriendlyStatus();
        log.printfln("|>  - Status: %d %s", status.first, status.second.c_str());

        PubSubClient &mqttClientRef = const_cast<PubSubClient&>(mqttClient);

        log.printfln("|>  - Buffer Size: %d", mqttClientRef.getBufferSize());
        log.printfln("|>  - Topic: %s", topic.c_str());

        // create a string with all the module callbacks
        String moduleCallbackNames = "";
        for (const auto& callback : moduleCallbacks) {
            moduleCallbackNames += callback.first + ", ";
        }
        Log.printfln("|>  - Module Callbacks: %s", moduleCallbackNames.c_str());
    };

    String MQTTController::getInfoForJson() const
    {
        JsonDocument doc = startJsonDoc();

        doc["clientName"] = getClientName();
        doc["desired"] = mqttConnectionDesired;
        doc["lastMessage"] = lastMessage;

        auto status = getFriendlyStatus();
        doc["status"]["code"] = status.first;
        doc["status"]["message"] = status.second;

        PubSubClient &mqttClientRef = const_cast<PubSubClient &>(mqttClient);
        doc["bufferSize"] = mqttClientRef.getBufferSize();
        doc["topic"] = topic;

        JsonArray callbacks = doc.createNestedArray("callbacks");
        for (const auto &callback : moduleCallbacks)
        {
            callbacks.add(callback.first);
        }

        String jsonString;
        serializeJson(doc, jsonString);
        return jsonString;
    }

    void MQTTController::setLastMessage(String message)
    {
        Log.println("MQTT: " + message);
        lastMessage = message;
    }

    String MQTTController::getLastMessage() const
    {
        return lastMessage;
    }


    int MQTTController::getStatus() const
    {
        return const_cast<PubSubClient&>(mqttClient).state();
    }


    std::pair<int, String> MQTTController::getFriendlyStatus() const
    {
        int statusID = const_cast<PubSubClient&>(mqttClient).state();
        String statusString;

        switch (statusID)
        {
            case MQTT_CONNECTION_TIMEOUT:
                statusString = "MQTT_CONNECTION_TIMEOUT";
                break;
            case MQTT_CONNECTION_LOST:
                statusString = "MQTT_CONNECTION_LOST";
                break;
            case MQTT_CONNECT_FAILED:
                statusString = "MQTT_CONNECT_FAILED";
                break;
            case MQTT_DISCONNECTED:
                statusString = "MQTT_DISCONNECTED";
                break;
            case MQTT_CONNECTED:
                statusString = "MQTT_CONNECTED";
                break;
            case MQTT_CONNECT_BAD_PROTOCOL:
                statusString = "MQTT_CONNECT_BAD_PROTOCOL";
                break;
            case MQTT_CONNECT_BAD_CLIENT_ID:
                statusString = "MQTT_CONNECT_BAD_CLIENT_ID";
                break;
            case MQTT_CONNECT_UNAVAILABLE:
                statusString = "MQTT_CONNECT_UNAVAILABLE";
                break;
            case MQTT_CONNECT_BAD_CREDENTIALS:
                statusString = "MQTT_CONNECT_BAD_CREDENTIALS";
                break;
            case MQTT_CONNECT_UNAUTHORIZED:
                statusString = "MQTT_CONNECT_UNAUTHORIZED";
                break;
            default:
                statusString = "UNKNOWN";
                break;
        }

        return std::make_pair(statusID, statusString);
    }

    const char* MQTTController::getClientName() const
    {
        SettingsCategory *wifiSettings = settingsManager.getCategory(WIFI_MODULE_NAME);
        return wifiSettings->getValue<String>("hostname").c_str();
    }


    void MQTTController::registerCallback(const String& moduleName, std::function<void(const String&, const String&)> callback) {
        moduleCallbacks[moduleName] = callback;
    }

#endif // ENABLE_MQTT
