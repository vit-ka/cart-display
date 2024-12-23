#include "bms_client.hpp"

BmsClient* BmsClient::instance = nullptr;

constexpr char BmsClient::SERVICE_UUID[];
constexpr char BmsClient::CHAR_NOTIFY[];
constexpr char BmsClient::CHAR_WRITE[];
constexpr uint8_t BmsClient::CMD_BASIC_INFO;

BmsClient::BmsClient(const char* address, DataCallback callback)
    : deviceAddress(address), dataCallback(callback) {
    instance = this;
}

void BmsClient::begin() {
    BLEDevice::init("");
    connectToServer();
}

void BmsClient::update() {
    if (!connected) {
        connectToServer();
    } else {
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
        connected = true;
        pRemoteService = pClient->getService(SERVICE_UUID);

        if(pRemoteService == nullptr) {
            connected = false;
            return;
        }

        pNotifyChar = pRemoteService->getCharacteristic(CHAR_NOTIFY);
        pWriteChar = pRemoteService->getCharacteristic(CHAR_WRITE);

        if (pNotifyChar == nullptr || pWriteChar == nullptr) {
            connected = false;
            return;
        }

        pNotifyChar->registerForNotify(notifyCallback);
    } else {
        connected = false;
    }
}