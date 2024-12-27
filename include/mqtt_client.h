#pragma once
#include <PubSubClient.h>
#include <WiFi.h>
#include "common_types.h"

class MqttClient {
public:
    static MqttClient& instance() {
        static MqttClient instance;
        return instance;
    }

    void setup(const char* ssid, const char* password,
               const char* mqtt_server, uint16_t mqtt_port, const char* mqtt_user, const char* mqtt_password);
    void update(const BmsData& data);
    void loop();

private:
    MqttClient() = default;
    WiFiClient espClient;
    PubSubClient client{espClient};
    uint32_t last_publish = 0;
    static constexpr uint32_t PUBLISH_INTERVAL = 60000;  // 1 minute
    const char* mqtt_server = nullptr;
    uint16_t mqtt_port = 0;
    const char* mqtt_user = nullptr;
    const char* mqtt_password = nullptr;

    bool reconnect();
    void publishMetric(const char* topic, float value);
    const char* getMQTTStateString(int state);
};