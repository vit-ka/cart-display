#include <string_view>

#include <Arduino.h>
#include "bms_client.h"
#include "common_types.h"

// Define the static constants here
static constexpr const char* SERVICE_UUID = "0000ff00-0000-1000-8000-00805f9b34fb";
static constexpr const char* CHAR_NOTIFY = "0000ff01-0000-1000-8000-00805f9b34fb";
static constexpr const char* CHAR_WRITE = "0000ff02-0000-1000-8000-00805f9b34fb";
static constexpr uint8_t CMD_BASIC_INFO = 0x03;

void BmsClient::notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    BmsClient::instance().decodeBmsData(pData, length);
}

BmsClient::BmsClient(const char* address, DataCallback dataCallback, StatusCallback statusCallback)
    : deviceAddress(address), dataCallback(dataCallback), statusCallback(statusCallback) {
}

void BmsClient::setup() {
    BLEDevice::init("");
    setConnectionState(ConnectionState::Connecting);
}

void BmsClient::update() {
    if (!isConnected()) {
        setConnectionState(ConnectionState::Connecting);
        connectToServer();
    } else {
        setConnectionState(ConnectionState::Connected);
        requestBmsData();
    }
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

                if (dataCallback) {
                    dataCallback(rawData);
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