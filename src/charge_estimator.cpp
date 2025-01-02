#include "charge_estimator.h"

#include <Arduino.h>

void ChargeEstimator::update(const BmsData& data, uint32_t timestamp) {
    bool is_charging = data.current > 0;

    if (!is_charging) {
        stopCharging();
        was_charging = false;
        last_update_ms = timestamp;
        last_soc = data.soc;
        return;
    }

    updateAccumulatedAh(data.current, timestamp);
    logSocChangeIfNeeded(data.soc, timestamp);

    if (!was_charging) {
        startCharging(data, timestamp);
    } else {
        updateChargingEstimates(data, timestamp);
    }

    was_charging = true;
    last_update_ms = timestamp;
    last_soc = data.soc;
}

void ChargeEstimator::updateAccumulatedAh(float current, uint32_t timestamp) {
    float hours = (timestamp - last_update_ms) / 1000.0f / 3600.0f;
    accumulated_ah += current * hours;
}

void ChargeEstimator::logSocChangeIfNeeded(uint16_t soc, uint32_t timestamp) {
    if (soc == last_logged_soc) return;

    ChargingLog::logDataPoint(timestamp, soc, accumulated_ah);
    accumulated_ah = 0.0f;
    last_logged_soc = soc;
}

void ChargeEstimator::startCharging(const BmsData& data, uint32_t timestamp) {
    charging_start_ms = timestamp;
    avg_charge_current = data.current;
    avg_charge_rate = 0.0f;
    last_soc = data.soc;
    time_to_full_s = 0;
}

void ChargeEstimator::updateChargingEstimates(const BmsData& data, uint32_t timestamp) {
    charging_duration_ms = timestamp - charging_start_ms;
    if (charging_duration_ms < REQUIRED_CHARGING_TIME_MS) return;

    // Update moving averages
    avg_charge_current = avg_charge_current * (1.0f - CHARGE_MA_FACTOR) + data.current * CHARGE_MA_FACTOR;

    float time_delta_h = (timestamp - last_update_ms) / 1000.0f / 3600.0f;
    if (time_delta_h > 0) {
        float instantaneous_rate = (data.soc - last_soc) / time_delta_h;
        avg_charge_rate = (avg_charge_rate == 0.0f)
                              ? instantaneous_rate
                              : avg_charge_rate * (1.0f - RATE_MA_FACTOR) + instantaneous_rate * RATE_MA_FACTOR;
    }

    time_to_full_s = calculateTimeToFull(data.soc, avg_charge_rate);
}

void ChargeEstimator::stopCharging() {
    charging_duration_ms = 0;
    time_to_full_s = 0;
    avg_charge_rate = 0.0f;
    accumulated_ah = 0.0f;
    last_logged_soc = static_cast<uint16_t>(last_soc);
}

float ChargeEstimator::calculateTimeToFull(float soc, float charge_rate) const {
    float remaining_soc = 100.0f - soc;

    if (charge_rate > 0.1f) {
        // Convert %/hour to %/second for consistent units
        float charge_rate_per_second = charge_rate / 3600.0f;

        // Time in seconds using both methods
        float current_based_seconds = (remaining_soc * BATTERY_CAPACITY_AH / 100.0f) / (avg_charge_current / 3600.0f);
        float soc_based_seconds = remaining_soc / charge_rate_per_second;

        float soc_weight = std::min(charging_duration_ms / 60000.0f, 0.8f);
        return soc_based_seconds * soc_weight + current_based_seconds * (1.0f - soc_weight);
    }

    // Initial estimate based on current only
    return (remaining_soc * BATTERY_CAPACITY_AH / 100.0f) / (avg_charge_current / 3600.0f);
}