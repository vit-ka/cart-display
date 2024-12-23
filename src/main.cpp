#include <Arduino.h>
#include "display_manager.h"
#include "bms_client.h"
#include "common_types.h"
#include "metrics_averager.h"

#define BATTERY_ADDRESS "a4:c1:37:03:f9:fc"

MetricsAverager averager;

void onBmsData(const BmsData& rawData) {
    uint32_t now = millis();
    averager.addMetrics(rawData, now);
    auto avgData = averager.getAverage();
    DisplayManager::instance().update(avgData);
}

void onConnectionStatus(ConnectionState status) {
    switch(status) {
        case ConnectionState::Connecting:
            Serial.println("Connecting to BMS...");
            DisplayManager::instance().updateConnectionState(ConnectionState::Connecting);
            break;
        case ConnectionState::Connected:
            Serial.println("Connected to BMS");
            DisplayManager::instance().updateConnectionState(ConnectionState::Connected);
            break;
    }
}

void setup() {
    Serial.begin(115200);
    DisplayManager::instance().setup();
    BmsClient::instance(BATTERY_ADDRESS, onBmsData, onConnectionStatus).setup();
}

void loop() {
    DisplayManager::instance().handleTasks();
    BmsClient::instance().update();
    delay(100);
}