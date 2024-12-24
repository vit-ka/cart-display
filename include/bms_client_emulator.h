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
        : dataCallback(dataCallback), statusCallback(statusCallback) {}

    DataCallback dataCallback;
    StatusCallback statusCallback;

    // Simulated battery state
    float voltage = 48.0f;      // 48V nominal
    float current = 0.0f;       // 0A initial
    uint16_t soc = 95.0f;         // 50% initial

    float accumulatedAmpHours = 0.0f;  // Track total amp-hours consumed

    // Simulation parameters
    static constexpr float MAX_CURRENT = 100.0f;  // ±100A
    static constexpr float MIN_VOLTAGE = 42.0f;   // 42V min
    static constexpr float MAX_VOLTAGE = 54.6f;   // 54.6V max

    bool connected = false;  // Track connection state
    uint32_t lastUpdate = 0;
    void simulateBatteryBehavior();


};