#pragma once

#include <deque>

#include "common_types.h"

class MetricsAverager {
   public:
    static constexpr uint32_t AVERAGE_WINDOW_MS = 3000;  // 3 seconds

    void addMetrics(const BmsData& data, uint32_t timestamp);
    BmsData getAverage() const;

   private:
    struct MetricsPoint {
        float voltage;
        float current;
        float power;
        uint16_t soc;
        uint32_t timestamp;
    };
    std::deque<MetricsPoint> history;
};