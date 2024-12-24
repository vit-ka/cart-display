#pragma once
#include "common_types.h"
#include "bms_interface.h"

class BmsClientEmulator : public IBmsClient {
public:
    static BmsClientEmulator& instance(const char* address = nullptr,
                                     DataCallback dataCallback = nullptr,
                                     StatusCallback statusCallback = nullptr) {
        static BmsClientEmulator instance(dataCallback, statusCallback);
        return instance;
    }

    void setup() override;
    void update() override;
    bool isConnected() const override { return connected; }

private:
    BmsClientEmulator(DataCallback dataCallback, StatusCallback statusCallback)
        : dataCallback(dataCallback), statusCallback(statusCallback) {
        // Initialize accumulatedAmpHours based on initial SOC
        accumulatedAmpHours = BATTERY_CAPACITY_AH * (1.0f - soc/100.0f);
    }

    DataCallback dataCallback;
    StatusCallback statusCallback;

    // Simulated battery state
    float voltage = 48.0f;      // 48V nominal
    float current = 0.0f;       // 0A initial
    uint16_t soc = 55.0f;         // 50% initial

    float accumulatedAmpHours = 0.0f;  // Will be initialized in constructor
    static constexpr float BATTERY_CAPACITY_AH = 100.0f;  // Move this to class scope

    // Simulation parameters
    static constexpr float MAX_CURRENT = 100.0f;  // ±100A
    static constexpr float MIN_VOLTAGE = 42.0f;   // 42V min
    static constexpr float MAX_VOLTAGE = 54.6f;   // 54.6V max

    bool connected = false;  // Track connection state
    uint32_t lastUpdate = 0;
    void simulateBatteryBehavior();


};