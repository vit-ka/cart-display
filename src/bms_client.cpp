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

void BmsClient::decodeBmsData(uint8_t* data, size_t length) {
    if (length < 4 || data[0] != 0xDD) return;

    switch(data[1]) {
        case 0x03:
            if (length >= 13) {
                BmsData bmsData;
                bmsData.voltage = (data[4] << 8 | data[5]) / 100.0f;
                bmsData.current = ((int16_t)(data[6] << 8 | data[7])) / 100.0f;
                bmsData.soc = (data[8] << 8 | data[9]) / 100;
                bmsData.power = bmsData.voltage * bmsData.current;

                if (dataCallback) {
                    dataCallback(bmsData);
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