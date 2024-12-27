#pragma once
#include <SPIFFS.h>

class ChargingLog {
   public:
    static bool init();

    // Add a new data point to the log
    static void logDataPoint(uint32_t timestamp_ms, uint16_t soc, float amp_hours);

    // Get the log file contents as a string
    static String getLogContents();

    // Clear the log file
    static void clearLog();

    static void startNewSession();

   private:
};