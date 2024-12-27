#include "charging_log.h"

#include <Arduino.h>

namespace {
    static constexpr const char* kLogFile = "/charging_log.tsv";
    static constexpr const char* kHeader = "timestamp_ms\tsoc\tamp_hours\n";
    static constexpr const char* kSessionSeparator =
        "\n=== New Charging Session ===\n"
        "# Note: First point's amp-hours are incomplete\n"
        "# timestamp_ms is milliseconds since device startup\n\n";



    // Helper function for common file operations
    File openLogFile(const char* mode) {
        if (!SPIFFS.exists(kLogFile) && mode[0] != 'w') {
            File file = SPIFFS.open(kLogFile, "w");
            if (!file) {
                Serial.println("Failed to create log file");
                return File();
            }
            file.print(kHeader);
            file.close();
        }

        File file = SPIFFS.open(kLogFile, mode);
        if (!file) {
            Serial.println("Failed to open log file");
        }
        return file;
    }
}

void ChargingLog::logDataPoint(uint32_t timestamp_ms, uint16_t soc, float amp_hours) {
    File file = openLogFile("a");
    if (!file) return;

    char buffer[48];
    snprintf(buffer, sizeof(buffer), "%u\t%u\t%.3f\n", timestamp_ms, soc, amp_hours);
    file.print(buffer);
    file.close();

    Serial.printf("Logged: SOC: %u%%, Accumulated Ah: %.3f\n", soc, amp_hours);
}

String ChargingLog::getLogContents() {
    if (!SPIFFS.exists(kLogFile)) {
        return "Log file not found";
    }

    File file = SPIFFS.open(kLogFile, "r");
    if (!file) {
        return "Failed to open log file";
    }

    String contents = file.readString();
    file.close();
    return contents;
}

void ChargingLog::clearLog() {
    if (SPIFFS.exists(kLogFile)) {
        SPIFFS.remove(kLogFile);
    }
}

void ChargingLog::startNewSession() {
    File file = openLogFile("a");
    if (!file) return;

    file.print(kSessionSeparator);
    file.close();

    Serial.println("Started new charging session");
}

bool ChargingLog::init() {
    return SPIFFS.begin(true);  // true = format on failure
}