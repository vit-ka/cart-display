#include <Arduino.h>
#include "display_manager.hpp"
#include "bms_client.hpp"

LGFX tft;
DisplayManager display;

#define BATTERY_ADDRESS "a4:c1:37:03:f9:fc"

void onBmsData(const BmsClient::BmsData& data) {
    display.update(data.voltage, data.current, data.power, data.soc);
}

void onConnectionStatus(BmsClient::ConnectionStatus status) {
    switch(status) {
        case BmsClient::ConnectionStatus::Connecting:
            Serial.println("Connecting to BMS...");
            display.updateConnectionState(DisplayManager::ConnectionState::Connecting);
            break;
        case BmsClient::ConnectionStatus::Connected:
            Serial.println("Connected to BMS");
            display.updateConnectionState(DisplayManager::ConnectionState::Connected);
            break;
        case BmsClient::ConnectionStatus::ServiceNotFound:
        case BmsClient::ConnectionStatus::CharacteristicsNotFound:
        case BmsClient::ConnectionStatus::DeviceNotFound:
        case BmsClient::ConnectionStatus::Disconnected:
            Serial.println("Connection failed or lost");
            display.updateConnectionState(DisplayManager::ConnectionState::Disconnected);
            break;
    }
}

BmsClient bms(BATTERY_ADDRESS, onBmsData, onConnectionStatus);

void setup() {
    Serial.begin(115200);

    // Initialize display first
    display.begin();
    display.updateConnectionState(DisplayManager::ConnectionState::Connecting);

    // Then start BLE connection
    bms.begin();
}

void loop() {
    display.handleTasks();  // Handle display updates first
    bms.update();          // Then handle BLE
    delay(100);
}