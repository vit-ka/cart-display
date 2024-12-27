#include "bms_client_emulator.h"
#include <Arduino.h>
#include <math.h>


// Simulation parameters
static constexpr float MAX_CURRENT = 300.0f;  // ±300A max
static constexpr float TYPICAL_CURRENT = -200.0f;  // Typical driving current
static constexpr float MIN_VOLTAGE = 42.0f;
static constexpr float MAX_VOLTAGE = 54.6f;

// Simulate disconnections every 10 seconds
static constexpr uint32_t DISCONNECT_INTERVAL = 10000;  // 10 seconds
static constexpr uint32_t DISCONNECT_DURATION = 2000;   // 2 seconds

// Add to top of file with other constants
static constexpr float OUTLET_CHARGING_CURRENT = 9.0f;  // 9A constant charging


void BmsClientEmulator::setup() {
    lastUpdate = millis();
    statusCallback(ConnectionState::Connected);
}

void BmsClientEmulator::update() {
    uint32_t now = millis();

    uint32_t cycleTime = now % (DISCONNECT_INTERVAL + DISCONNECT_DURATION);
    bool shouldBeConnected = cycleTime < DISCONNECT_INTERVAL;

    if (shouldBeConnected != isConnected()) {
        if (shouldBeConnected) {
            statusCallback(ConnectionState::Connected);
        } else {
            statusCallback(ConnectionState::Connecting);
        }
        connected = shouldBeConnected;
    }

    // Only send data when connected
    if (connected && now - lastUpdate >= 100) {
        simulateBatteryBehavior();
        lastUpdate = now;
    }
}

void BmsClientEmulator::simulateBatteryBehavior() {
    static float time = 0;
    time += 0.5f;

    uint32_t now = millis();
    float timeStep = (now - lastUpdate) / 1000.0f / 3600.0f;
    lastUpdate = now;

#ifdef EMULATE_OUTLET_CHARGING
    current = OUTLET_CHARGING_CURRENT;
    voltage = MIN_VOLTAGE + (MAX_VOLTAGE - MIN_VOLTAGE) * (soc / 100.0f);
#else
    // Driving/regen behavior
    float baseWave = -0.2f - 0.1f * sin(time * 0.5f);
    float randomNoise = (random(100) - 50) / 300.0f;

    // Handle regenerative braking state
    static bool inRegenState = false;
    static uint32_t regenStartTime = 0;
    if (!inRegenState && random(100) < 3) {  // 3% chance to start regen
        inRegenState = true;
        regenStartTime = now;
        baseWave = 0.3f;  // Strong initial regen
    } else if (inRegenState) {
        if (now - regenStartTime < 2000) {  // Maintain regen for 2 seconds
            baseWave = 0.3f - (now - regenStartTime) / 8000.0f;  // Gradually decrease regen
        } else {
            inRegenState = false;
        }
    }

    // Add acceleration spikes only when not in regen
    if (!inRegenState && random(100) < 15) {
        randomNoise = 0.15f + random(20) / 200.0f;
    }

    // Calculate base current
    current = TYPICAL_CURRENT * (baseWave + randomNoise);

    // Limit current draw based on SOC (reduce power when battery is low)
    float socFactor = soc / 100.0f;
    float currentLimit = TYPICAL_CURRENT * (0.2f + 0.8f * socFactor);  // 20% at 0% SOC, 100% at 100% SOC
    current = constrain(current, -currentLimit, currentLimit);

    // Calculate voltage based on SOC and current
    float baseVoltage = 42.0f + (52.0f - 42.0f) * (soc / 100.0f);
    float voltageNoise = (random(100) - 50) / 250.0f;
    float voltageOffset = (current / MAX_CURRENT) * 3.0f;
    voltage = baseVoltage + voltageOffset + voltageNoise;
    voltage = constrain(voltage, MIN_VOLTAGE, MAX_VOLTAGE);
#endif

    accumulatedAmpHours += current * timeStep;
    soc += (current * timeStep / BATTERY_CAPACITY_AH) * 100.0f;
    soc = constrain(soc, 0.0f, 100.0f);

    // More realistic latency simulation
    static uint32_t base_latency = 100;
    if (random(100) < 10) {  // 10% chance to spike
        base_latency = random(200, 2000);  // Occasional big spikes
    } else if (random(100) < 30) {  // 30% chance to drift
        base_latency += random(-20, 20);
        base_latency = constrain(base_latency, 50, 500);
    }
    uint32_t latency = base_latency + random(20);  // Add jitter

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