#include "mqtt_client.h"
#include <Arduino.h>

void MqttClient::setup(const char* ssid, const char* password,
                      const char* mqtt_server, uint16_t mqtt_port,
                      const char* mqtt_user, const char* mqtt_password) {
    this->mqtt_server = mqtt_server;
    this->mqtt_port = mqtt_port;
    this->mqtt_user = mqtt_user;
    this->mqtt_password = mqtt_password;
    Serial.printf("Connecting to WiFi: %s\n", ssid);
    WiFi.begin(ssid, password);

    uint8_t wifi_retry = 0;
    while (WiFi.status() != WL_CONNECTED && wifi_retry < 20) {
        delay(500);
        Serial.print(".");
        wifi_retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nWiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
        client.setServer(mqtt_server, mqtt_port);
        Serial.printf("MQTT server set to: %s:%d\n", mqtt_server, mqtt_port);
        last_publish = 0;
    } else {
        Serial.println("\nWiFi connection failed!");
    }
}

void MqttClient::update(const BmsData& data) {
    uint32_t now = millis();
    if (now - last_publish < PUBLISH_INTERVAL && last_publish != 0) {
        return;
    }

    if (!client.connected()) {
        Serial.println("MQTT not connected, attempting reconnect...");
        reconnect();
        if (!client.connected()) {
            Serial.println("MQTT still not connected, skipping publish");
            return;
        }
    }

    Serial.printf("Publishing MQTT update: SOC=%d%%\n", data.soc);
    if (client.publish("homeassistant/sensor/cart_battery/soc", String(data.soc).c_str(), true)) {
        Serial.println("Published successfully");
        last_publish = now;
    } else {
        Serial.println("Publish failed");
    }
}

void MqttClient::loop() {
    if (!client.connected()) {
        Serial.printf("MQTT disconnected, state=%d\n", client.state());
    }
    client.loop();
}

bool MqttClient::reconnect() {
    static uint8_t retry_count = 0;
    const uint8_t MAX_RETRIES = 3;
    retry_count = 0;

    while (!client.connected() && retry_count < MAX_RETRIES) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected. IP: " + WiFi.localIP().toString());
            return false;
        }

        String clientId = "CartBatteryClient-";
        clientId += String(random(0xffff), HEX);

        IPAddress mqtt_ip;
        if (!WiFi.hostByName(mqtt_server, mqtt_ip)) {
            Serial.println("DNS lookup failed for MQTT server");
            retry_count++;
            delay(1000);
            continue;
        }
        Serial.printf("MQTT server resolved to: %s\n", mqtt_ip.toString().c_str());

        client.setServer(mqtt_ip, mqtt_port);

        Serial.printf("Attempting MQTT connection to %s:%d as %s (try %d/%d)...",
                     mqtt_ip.toString().c_str(), mqtt_port, clientId.c_str(),
                     retry_count + 1, MAX_RETRIES);

        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            if (client.subscribe("homeassistant/sensor/cart_battery/test")) {
                Serial.println("Subscribed to test topic");
            } else {
                Serial.println("Failed to subscribe");
            }
            return true;
        }

        Serial.printf("failed, rc=%d (%s)\n", client.state(), getMQTTStateString(client.state()));
        retry_count++;
        delay(1000);
    }

    return false;
}

void MqttClient::publishMetric(const char* topic, float value) {
    char msg[32];
    snprintf(msg, sizeof(msg), "%.1f", value);
    client.publish(topic, msg, true);
    Serial.printf("Published to MQTT: %s = %s\n", topic, msg);
}

const char* MqttClient::getMQTTStateString(int state) {
    switch (state) {
        case -4: return "MQTT_CONNECTION_TIMEOUT";
        case -3: return "MQTT_CONNECTION_LOST";
        case -2: return "MQTT_CONNECT_FAILED";
        case -1: return "MQTT_DISCONNECTED";
        case 0: return "MQTT_CONNECTED";
        case 1: return "MQTT_CONNECT_BAD_PROTOCOL";
        case 2: return "MQTT_CONNECT_BAD_CLIENT_ID";
        case 3: return "MQTT_CONNECT_UNAVAILABLE";
        case 4: return "MQTT_CONNECT_BAD_CREDENTIALS";
        case 5: return "MQTT_CONNECT_UNAUTHORIZED";
        default: return "UNKNOWN";
    }
}