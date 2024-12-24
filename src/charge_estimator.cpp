#include "charge_estimator.h"
#include <Arduino.h>

void ChargeEstimator::update(const BmsData& data, uint32_t timestamp) {
    bool is_charging = data.current > 0;

    if (is_charging && !was_charging) {
        // Just started charging
        charging_start_ms = timestamp;
        avg_charge_current = data.current;
        time_to_full_s = 0;
    } else if (is_charging) {
        // Update charging duration
        charging_duration_ms = timestamp - charging_start_ms;

        // After required time, start estimating
        if (charging_duration_ms >= REQUIRED_CHARGING_TIME_MS) {
            // Update running average of charge current
            avg_charge_current = avg_charge_current * (1.0f - CHARGE_MA_FACTOR) +
                               data.current * CHARGE_MA_FACTOR;

            // Calculate time to full
            float remaining_ah = 100.0f * (1.0f - data.soc/100.0f);
            float hours_to_full = remaining_ah / avg_charge_current;
            time_to_full_s = hours_to_full * 3600;
        }
    } else {
        // Reset when not charging
        charging_duration_ms = 0;
        time_to_full_s = 0;
    }

    was_charging = is_charging;
    last_update_ms = timestamp;
}