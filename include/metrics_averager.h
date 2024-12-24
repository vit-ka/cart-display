#pragma once
#include <deque>
#include <type_traits>

#include "common_types.h"

class MetricsAverager {
   public:
    void addMetrics(const BmsData& data, uint32_t timestamp);
    [[nodiscard]] BmsData getAverage() const;

   private:
    struct MetricsPoint {
        float voltage;
        float current;
        float power;
        uint16_t soc;
        uint32_t timestamp;
    };

    template <typename T, typename Selector>
    [[nodiscard]] static constexpr T average(const std::deque<MetricsPoint>& range, Selector selector) {
        if (range.empty()) return T{};

        T sum{};
        for (const auto& point : range) {
            sum += selector(point);
        }
        return sum / static_cast<T>(range.size());
    }

    std::deque<MetricsPoint> history;
};