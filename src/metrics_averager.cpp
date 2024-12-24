#include "metrics_averager.h"

#include <Arduino.h>

void MetricsAverager::addMetrics(const BmsData& data, uint32_t timestamp) {
    while (!history.empty() && timestamp - history.front().timestamp > AVERAGE_WINDOW_MS) {
        history.pop_front();
    }

    MetricsPoint point = {
        .voltage = data.voltage, .current = data.current, .power = data.power, .soc = data.soc, .timestamp = timestamp};
    history.push_back(point);
}

BmsData MetricsAverager::getAverage() const {
    BmsData avg = {0, 0, 0, 0, 0};
    if (history.empty()) return avg;

    float sum_voltage = 0;
    float sum_current = 0;
    float sum_power = 0;
    uint32_t sum_soc = 0;

    for (const auto& point : history) {
        sum_voltage += point.voltage;
        sum_current += point.current;
        sum_power += point.power;
        sum_soc += point.soc;
    }

    size_t count = history.size();
    avg.voltage = sum_voltage / count;
    avg.current = sum_current / count;
    avg.power = sum_power / count;
    avg.soc = sum_soc / count;

    // Calculate average time between last 5 points
    if (count >= 2) {
        size_t latency_points = std::min(size_t(5), count - 1);
        uint32_t sum_intervals = 0;
        auto it = history.end();
        auto prev = --it;  // Point to last element

        for (size_t i = 0; i < latency_points; ++i) {
            --it;  // Move to previous element
            sum_intervals += prev->timestamp - it->timestamp;
            prev = it;
        }
        avg.latency_ms = sum_intervals / latency_points;
    }

    return avg;
}