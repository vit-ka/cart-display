#include "bms_client_emulator.h"
#include <Arduino.h>
#include <math.h>

void BmsClientEmulator::setup() {
    lastUpdate = millis();
    statusCallback(ConnectionState::Connected);
}

void BmsClientEmulator::update() {
    uint32_t now = millis();
    if (now - lastUpdate >= 200) {  // Update every 200ms
        simulateBatteryBehavior();
        lastUpdate = now;
    }
}

void BmsClientEmulator::simulateBatteryBehavior() {
    static float time = 0;
    time += 0.1f;

    // Base current on sine wave with random variations
    float baseWave = sin(time * 0.1f);
    float randomNoise = (random(100) - 50) / 100.0f;  // Random value between -0.5 and 0.5
    current = MAX_CURRENT * (baseWave + randomNoise * 0.3f);  // Add 30% random variation

    // Add some voltage noise
    float voltageNoise = (random(100) - 50) / 500.0f;  // ±0.1V noise
    float voltageOffset = (current / MAX_CURRENT) * 2.0f;  // ±2V based on current
    voltage = 48.0f + voltageOffset + voltageNoise;
    voltage = constrain(voltage, MIN_VOLTAGE, MAX_VOLTAGE);

    // More dynamic SOC changes
    float socNoise = (random(100) - 50) / 1000.0f;  // Small random SOC variations
    float socChange = (current * 0.01f) + socNoise;
    soc = constrain(soc + socChange, 0, 100);

    // Simulate variable latency
    uint32_t latency = 30 + random(40);  // 30-70ms latency

    if (dataCallback) {
        BmsData data = {
            .voltage = voltage,
            .current = current,
            .power = voltage * current,
            .soc = static_cast<uint16_t>(soc),
            .latency_ms = latency
        };
        dataCallback(data);
    }
}