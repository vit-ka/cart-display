#include "metrics_averager.h"

#include <algorithm>

namespace {
static constexpr uint32_t kAverageWindowMs = 1000;
}

void MetricsAverager::addMetrics(const BmsData& data, uint32_t timestamp) {
    // Remove old entries
    while (!history.empty() && timestamp - history.front().timestamp > kAverageWindowMs) {
        history.pop_front();
    }

    history.push_back({.voltage = data.voltage,
                       .current = data.current,
                       .power = data.power,
                       .soc = data.soc,
                       .timestamp = timestamp});
}

BmsData MetricsAverager::getAverage() const {
    if (history.empty()) return {};

    BmsData avg{.voltage = average<float>(history, [](const MetricsPoint& p) { return p.voltage; }),
                .current = average<float>(history, [](const MetricsPoint& p) { return p.current; }),
                .power = average<float>(history, [](const MetricsPoint& p) { return p.power; }),
                .soc = static_cast<uint16_t>(average<float>(history, [](const MetricsPoint& p) { return p.soc; }))};

    // Calculate average latency from last 5 points
    if (history.size() >= 2) {
        size_t latency_points = std::min(size_t{5}, history.size() - 1);
        uint32_t sum_intervals = 0;
        auto it = history.end();
        auto prev = --it;

        for (size_t i = 0; i < latency_points; ++i) {
            --it;
            sum_intervals += prev->timestamp - it->timestamp;
            prev = it;
        }
        avg.latency_ms = sum_intervals / latency_points;
    }

    return avg;
}