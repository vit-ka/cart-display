#include "bms_client_emulator.h"
#include <Arduino.h>
#include <math.h>

void BmsClientEmulator::setup() {
    lastUpdate = millis();
    statusCallback(ConnectionState::Connected);
}

void BmsClientEmulator::update() {
    uint32_t now = millis();
    if (now - lastUpdate >= 100) {  // Update every 100ms for faster response
        simulateBatteryBehavior();
        lastUpdate = now;
    }
}

void BmsClientEmulator::simulateBatteryBehavior() {
    static float time = 0;
    time += 0.5f;  // Faster time progression

    // Simulate rapid current changes (acceleration/regen)
    float baseWave = -sin(time * 0.5f);  // Inverted sine wave for correct direction
    float randomNoise = (random(100) - 50) / 100.0f;
    // Add sudden spikes for acceleration (negative current)
    if (random(100) < 10) {  // 10% chance of acceleration spike
        randomNoise = 0.8f + random(20) / 100.0f;  // Strong discharge (0.8 to 1.0)
    }
    // Add regenerative braking spikes (positive current)
    if (random(100) < 5) {   // 5% chance of regen spike
        randomNoise = -0.4f - random(20) / 100.0f;   // Moderate regen (-0.4 to -0.6)
    }

    current = MAX_CURRENT * (baseWave + randomNoise);

    // More responsive voltage changes
    float voltageNoise = (random(100) - 50) / 250.0f;  // ±0.2V noise
    float voltageOffset = (current / MAX_CURRENT) * 3.0f;  // ±3V based on current (more voltage sag)
    voltage = 48.0f + voltageOffset + voltageNoise;
    voltage = constrain(voltage, MIN_VOLTAGE, MAX_VOLTAGE);

    // Faster SOC changes
    float socNoise = (random(100) - 50) / 500.0f;
    float socChange = (current * 0.02f) + socNoise;  // Doubled SOC change rate
    soc = constrain(soc + socChange, 0, 100);

    // More variable latency during high current
    uint32_t latency = 30 + random(40) + abs(current) / 2;  // Higher latency under load

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