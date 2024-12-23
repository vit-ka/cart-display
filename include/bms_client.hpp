#pragma once

#include <BLEDevice.h>
#include <BLEClient.h>
#include <deque>
#include "common_types.hpp"

class BmsClient {
public:
    // Data structures
    struct BmsData {
        float voltage;
        float current;
        float power;
        uint16_t soc;
    };

    // Callback types
    using DataCallback = std::function<void(const BmsData&)>;
    using StatusCallback = std::function<void(ConnectionState)>;

    // Constants
    static constexpr uint32_t AVERAGE_WINDOW_MS = 3000;  // 3 seconds in milliseconds
    static constexpr char SERVICE_UUID[] = "0000ff00-0000-1000-8000-00805f9b34fb";
    static constexpr char CHAR_NOTIFY[] = "0000ff01-0000-1000-8000-00805f9b34fb";
    static constexpr char CHAR_WRITE[] = "0000ff02-0000-1000-8000-00805f9b34fb";
    static constexpr uint8_t CMD_BASIC_INFO = 0x03;

    // Public interface
    BmsClient(const char* address, DataCallback dataCallback, StatusCallback statusCallback);
    void begin();
    void update();
    bool isConnected() const { return connected; }

private:
    // BLE related
    static BmsClient* instance;
    const char* deviceAddress;
    BLEClient* pClient = nullptr;
    BLERemoteService* pRemoteService = nullptr;
    BLERemoteCharacteristic* pNotifyChar = nullptr;
    BLERemoteCharacteristic* pWriteChar = nullptr;
    bool connected = false;

    // Callbacks
    DataCallback dataCallback;
    StatusCallback statusCallback;

    // Data averaging
    struct PowerMetrics {
        float voltage;
        float current;
        float power;
        uint16_t soc;
        uint32_t timestamp;
    };
    std::deque<PowerMetrics> metrics_history;

    // BLE methods
    static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
    void connectToServer();
    void requestBmsData();
    void setConnectionState(ConnectionState state) {
        statusCallback(state);
        connected = state == ConnectionState::Connected;
    }

    // Data processing methods
    void decodeBmsData(uint8_t* data, size_t length);
    BmsData calculateAverage();
    void addMetricsToHistory(const BmsData& data);
};