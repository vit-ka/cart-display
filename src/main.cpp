#include <Arduino.h>
#include "display_manager.h"
#include "bms_client.h"
#include "common_types.h"

LGFX tft;
DisplayManager display;

#define BATTERY_ADDRESS "a4:c1:37:03:f9:fc"

void onBmsData(const BmsData& data) {
    display.update(data);
}

void onConnectionStatus(ConnectionState status) {
    switch(status) {
        case ConnectionState::Connecting:
            Serial.println("Connecting to BMS...");
            display.updateConnectionState(ConnectionState::Connecting);
            break;
        case ConnectionState::Connected:
            Serial.println("Connected to BMS");
            display.updateConnectionState(ConnectionState::Connected);
            break;
    }
}

BmsClient bms(BATTERY_ADDRESS, onBmsData, onConnectionStatus);

void setup() {
    Serial.begin(115200);

    // Initialize display first
    display.begin();
    display.updateConnectionState(ConnectionState::Connecting);

    // Then start BLE connection
    bms.begin();
}

void loop() {
    display.handleTasks();  // Handle display updates first
    bms.update();          // Then handle BLE
    delay(100);
}