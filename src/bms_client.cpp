#include <Arduino.h>
#include "bms_client.hpp"
#include "common_types.hpp"

BmsClient* BmsClient::instance = nullptr;

constexpr char BmsClient::SERVICE_UUID[];
constexpr char BmsClient::CHAR_NOTIFY[];
constexpr char BmsClient::CHAR_WRITE[];
constexpr uint8_t BmsClient::CMD_BASIC_INFO;

BmsClient::BmsClient(const char* address, DataCallback dataCallback, StatusCallback statusCallback)
    : deviceAddress(address), dataCallback(dataCallback), statusCallback(statusCallback) {
    instance = this;
}

void BmsClient::begin() {
    BLEDevice::init("");
    setConnectionState(ConnectionState::Connecting);
}

void BmsClient::update() {
    static bool first_update = true;

    if (first_update) {
        first_update = false;
    }

    if (!connected) {
        setConnectionState(ConnectionState::Connecting);
        connectToServer();
    } else {
        setConnectionState(ConnectionState::Connected);
        requestBmsData();
    }
}

void BmsClient::notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (instance) {
        instance->decodeBmsData(pData, length);
    }
}

void BmsClient::addMetricsToHistory(const BmsData& data) {
    uint32_t current_time = millis();
    while (!metrics_history.empty() && current_time - metrics_history.front().timestamp > AVERAGE_WINDOW_MS) {
        metrics_history.pop_front();
    }

    PowerMetrics metrics = {
        .voltage = data.voltage,
        .current = data.current,
        .power = data.power,
        .soc = data.soc,
        .timestamp = current_time
    };
    metrics_history.push_back(metrics);
}

BmsData BmsClient::calculateAverage() {
    BmsData avg = {0, 0, 0, 0, 0};
    if (metrics_history.empty()) return avg;

    float sum_voltage = 0;
    float sum_current = 0;
    float sum_power = 0;
    uint32_t sum_soc = 0;
    uint32_t sum_latency = 0;
    uint32_t current_time = millis();

    for (const auto& metrics : metrics_history) {
        sum_voltage += metrics.voltage;
        sum_current += metrics.current;
        sum_power += metrics.power;
        sum_soc += metrics.soc;
    }

    size_t count = metrics_history.size();
    avg.voltage = sum_voltage / count;
    avg.current = sum_current / count;
    avg.power = sum_power / count;
    avg.soc = sum_soc / count;

    // Calculate average latency from last 5 points
    size_t latency_points = std::min(size_t(5), count);
    auto it = metrics_history.end();
    for (size_t i = 0; i < latency_points; ++i) {
        --it;
        sum_latency += current_time - it->timestamp;
    }
    avg.latency_ms = sum_latency / latency_points;

    return avg;
}

void BmsClient::decodeBmsData(uint8_t* data, size_t length) {
    if (length < 4 || data[0] != 0xDD) return;

    switch(data[1]) {
        case 0x03:
            if (length >= 13) {
                BmsData rawData;
                rawData.voltage = (data[4] << 8 | data[5]) / 100.0f;
                rawData.current = ((int16_t)(data[6] << 8 | data[7])) / 100.0f;
                rawData.soc = (data[8] << 8 | data[9]) / 100;
                rawData.power = rawData.voltage * rawData.current;

                // Add to history and calculate average
                addMetricsToHistory(rawData);
                BmsData avgData = calculateAverage();

                if (dataCallback) {
                    dataCallback(avgData);
                }
            }
            break;
    }
}

void BmsClient::requestBmsData() {
    if (pWriteChar == nullptr) return;
    uint8_t cmd[7] = {0xDD, 0xA5, CMD_BASIC_INFO, 0x00, 0xFF, 0xFD, 0x77};
    pWriteChar->writeValue(cmd, sizeof(cmd));
}

void BmsClient::connectToServer() {
    if (pClient != nullptr) {
        delete pClient;
        pClient = nullptr;
    }

    pClient = BLEDevice::createClient();

    if(pClient->connect(BLEAddress(deviceAddress))) {
        pRemoteService = pClient->getService(SERVICE_UUID);
        if(pRemoteService == nullptr) {
            setConnectionState(ConnectionState::Connecting);
            return;
        }

        pNotifyChar = pRemoteService->getCharacteristic(CHAR_NOTIFY);
        pWriteChar = pRemoteService->getCharacteristic(CHAR_WRITE);

        if (pNotifyChar == nullptr || pWriteChar == nullptr) {
            setConnectionState(ConnectionState::Connecting);
            return;
        }

        pNotifyChar->registerForNotify(notifyCallback);
        setConnectionState(ConnectionState::Connected);
    } else {
        setConnectionState(ConnectionState::Connecting);
    }
}