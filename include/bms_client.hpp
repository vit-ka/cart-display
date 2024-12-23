#pragma once

#include <BLEDevice.h>
#include <BLEClient.h>

class BmsClient {
public:
    struct BmsData {
        float voltage;
        float current;
        float power;
        uint16_t soc;
    };

    enum class ConnectionStatus {
        Connecting,
        Connected,
        ServiceNotFound,
        CharacteristicsNotFound,
        DeviceNotFound,
        Disconnected
    };

    using DataCallback = std::function<void(const BmsData&)>;
    using StatusCallback = std::function<void(ConnectionStatus)>;

    BmsClient(const char* address, DataCallback dataCallback, StatusCallback statusCallback);
    void begin();
    void update();
    bool isConnected() const { return connected; }

private:
    static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
    void connectToServer();
    void decodeBmsData(uint8_t* data, size_t length);
    void requestBmsData();

    const char* deviceAddress;
    DataCallback dataCallback;
    BLEClient* pClient = nullptr;
    bool connected = false;
    BLERemoteService* pRemoteService = nullptr;
    BLERemoteCharacteristic* pNotifyChar = nullptr;
    BLERemoteCharacteristic* pWriteChar = nullptr;

    static constexpr char SERVICE_UUID[] = "0000ff00-0000-1000-8000-00805f9b34fb";
    static constexpr char CHAR_NOTIFY[] = "0000ff01-0000-1000-8000-00805f9b34fb";
    static constexpr char CHAR_WRITE[] = "0000ff02-0000-1000-8000-00805f9b34fb";
    static constexpr uint8_t CMD_BASIC_INFO = 0x03;

    static BmsClient* instance;
    StatusCallback statusCallback;
};