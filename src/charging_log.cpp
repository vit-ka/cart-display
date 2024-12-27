#include "charging_log.h"

void ChargingLog::logDataPoint(uint32_t timestamp_ms, float soc, float amp_hours, float power) {
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

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%u\t%.2f\t%.3f\t%.1f\n",
             timestamp_ms, soc, amp_hours, power);
    file.print(buffer);
    file.close();
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