#include "charging_log.h"

#include <Arduino.h>

namespace {
static constexpr const char* kLogFile = "/charging_log.tsv";
static constexpr const char* kHeader =
    "timestamp_ms\tsoc\tamp_hours\n"
    "# Note: First two point's amp-hours are incomplete\n"
    "# timestamp_ms is milliseconds since device startup\n";
static constexpr const char* kSessionSeparator = "\n";
static constexpr size_t kMaxLogSize = 32 * 1024;    // 32KB max
static constexpr size_t kReservedSpace = 4 * 1024;  // Keep 4KB free

bool checkSpaceAvailable() {
    size_t total = SPIFFS.totalBytes();
    size_t used = SPIFFS.usedBytes();
    size_t free = total - used;

    Serial.printf("SPIFFS: Total: %u bytes, Used: %u bytes, Free: %u bytes\n", total, used, free);

    if (free < kReservedSpace) {
        Serial.printf("Low storage space: %u bytes free\n", free);
        return false;
    }
    return true;
}

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

void truncateLogIfNeeded() {
    if (!SPIFFS.exists(kLogFile)) return;

    File file = SPIFFS.open(kLogFile, "r");
    if (!file) return;

    if (file.size() > kMaxLogSize) {
        Serial.printf("%s file too large, keeping only newest records...\n", kLogFile);
        String contents = file.readString();
        file.close();

        // Find lines from the end to keep half the max size
        size_t targetSize = kMaxLogSize / 2;
        size_t pos = contents.length();
        size_t newlines = 0;

        while (pos > 0 && (contents.length() - pos) < targetSize) {
            pos--;
            if (contents[pos] == '\n') newlines++;
        }

        // Find start of a complete line
        while (pos > 0 && contents[pos] != '\n') pos--;
        if (pos > 0) pos++;  // Skip the newline

        String newContents = contents.substring(pos);

        File wfile = openLogFile("w");
        if (wfile) {
            wfile.print(kHeader);
            wfile.print("# Log truncated, keeping newest ");
            wfile.print(newlines);
            wfile.print(" records\n");
            wfile.print(newContents);
            wfile.close();
        }
    } else {
        file.close();
    }
}
}  // namespace

void ChargingLog::logDataPoint(uint32_t timestamp_ms, uint16_t soc, float amp_hours) {
    if (!checkSpaceAvailable()) {
        truncateLogIfNeeded();
        if (!checkSpaceAvailable()) {
            Serial.println("Still no space available after truncation, skipping log");
            return;
        }
    }

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