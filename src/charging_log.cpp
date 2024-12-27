#include "charging_log.h"

#include <Arduino.h>

void ChargingLog::logDataPoint(uint32_t timestamp_ms, float soc, float amp_hours) {
    if (!SPIFFS.exists(LOG_FILE)) {
        File file = SPIFFS.open(LOG_FILE, "w");
        if (!file) {
            Serial.println("Failed to create log file");
            return;
        }
        file.print(HEADER);
        file.close();
    }

    File file = SPIFFS.open(LOG_FILE, "a");
    if (!file) {
        Serial.println("Failed to open log file for append");
        return;
    }

    char buffer[48];
    snprintf(buffer, sizeof(buffer), "%u\t%.2f\t%.3f\n", timestamp_ms, soc, amp_hours);
    file.print(buffer);
    file.close();

    Serial.printf("Logged: SOC: %.1f%%, Accumulated Ah: %.3f\n", soc, amp_hours);
}

String ChargingLog::getLogContents() {
    if (!SPIFFS.exists(LOG_FILE)) {
        return "Log file not found";
    }

    File file = SPIFFS.open(LOG_FILE, "r");
    if (!file) {
        return "Failed to open log file";
    }

    String contents = file.readString();
    file.close();
    return contents;
}

void ChargingLog::clearLog() {
    if (SPIFFS.exists(LOG_FILE)) {
        SPIFFS.remove(LOG_FILE);
    }
}