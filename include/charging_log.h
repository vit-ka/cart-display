#pragma once
#include <SPIFFS.h>
#include "common_types.h"

class ChargingLog {
public:
    static bool init() {
        return SPIFFS.begin(true);  // true = format on failure
    }

    // Add a new data point to the log
    static void logDataPoint(uint32_t timestamp_ms, float soc, float amp_hours, float power);

    // Get the log file contents as a string
    static String getLogContents();

    // Clear the log file
    static void clearLog();

private:
    static constexpr const char* LOG_FILE = "/charging_log.tsv";
    static constexpr const char* HEADER = "timestamp_ms\tsoc\tamp_hours\tpower\n";
};