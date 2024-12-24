#pragma once
#include "common_types.h"

class ChargeEstimator {
public:
    void update(const BmsData& data, uint32_t timestamp);
    uint32_t getTimeToFullCharge() const { return time_to_full_s; }
    bool isEstimating() const { return charging_duration_ms >= REQUIRED_CHARGING_TIME_MS; }

private:
    static constexpr uint32_t REQUIRED_CHARGING_TIME_MS = 10000;  // 10 seconds
    static constexpr float CHARGE_MA_FACTOR = 0.05f;  // Moving average factor

    uint32_t last_update_ms = 0;
    uint32_t charging_start_ms = 0;
    uint32_t charging_duration_ms = 0;
    float avg_charge_current = 0.0f;
    uint32_t time_to_full_s = 0;
    bool was_charging = false;
};