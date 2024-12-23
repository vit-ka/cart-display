#include <Arduino.h>
#include "display_manager.hpp"
#include "bms_client.hpp"

LGFX tft;
DisplayManager display;

#define BATTERY_ADDRESS "a4:c1:37:03:f9:fc"

void onBmsData(const BmsClient::BmsData& data) {
    display.update(data.voltage, data.current, data.power, data.soc);
}

BmsClient bms(BATTERY_ADDRESS, onBmsData);

void setup() {
    Serial.begin(115200);
    display.begin();
    bms.begin();
}

void loop() {
    bms.update();
    display.handleTasks();
    delay(100);
}