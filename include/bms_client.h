#pragma once
#include "bms_interface.h"

#include <deque>

#include <BLEClient.h>
#include <BLEDevice.h>

#include "common_types.h"

class BmsClient: public IBmsClient {
   public:
    // Callback types
    using DataCallback = std::function<void(const BmsData &)>;
    using StatusCallback = std::function<void(ConnectionState)>;

    // Constants
    static constexpr uint32_t AVERAGE_WINDOW_MS = 3000;  // 3 seconds in milliseconds

    // Delete copy and move operations
    BmsClient(const BmsClient &) = delete;
    BmsClient &operator=(const BmsClient &) = delete;
    BmsClient(BmsClient &&) = delete;
    BmsClient &operator=(BmsClient &&) = delete;

    // Singleton access
    static BmsClient &instance(const char *address = nullptr, DataCallback dataCallback = nullptr,
                               StatusCallback statusCallback = nullptr) {
        static BmsClient instance(address, dataCallback, statusCallback);
        return instance;
    }

    // Public interface
    void setup() override;
    void update() override;
    bool isConnected() const override {
        return pClient != nullptr && pRemoteService != nullptr && pNotifyChar != nullptr && pWriteChar != nullptr;
    }

   private:
    // Make constructor private
    BmsClient(const char *address, DataCallback dataCallback, StatusCallback statusCallback);

    // BLE related
    const char *deviceAddress;
    BLEClient *pClient = nullptr;
    BLERemoteService *pRemoteService = nullptr;
    BLERemoteCharacteristic *pNotifyChar = nullptr;
    BLERemoteCharacteristic *pWriteChar = nullptr;

    // Callbacks
    DataCallback dataCallback;
    StatusCallback statusCallback;

    // BLE methods
    static void notifyCallback(BLERemoteCharacteristic *pChar, uint8_t *pData, size_t length, bool isNotify);
    void connectToServer();
    void requestBmsData();

    // Data processing methods
    void decodeBmsData(uint8_t *data, size_t length);
    BmsData calculateAverage();
    void addMetricsToHistory(const BmsData &data);
};