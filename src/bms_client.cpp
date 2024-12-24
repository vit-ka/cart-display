#include "bms_client.h"

#include <Arduino.h>

#include "common_types.h"

// Define the static constants here
static constexpr const char *SERVICE_UUID = "0000ff00-0000-1000-8000-00805f9b34fb";
static constexpr const char *CHAR_NOTIFY = "0000ff01-0000-1000-8000-00805f9b34fb";
static constexpr const char *CHAR_WRITE = "0000ff02-0000-1000-8000-00805f9b34fb";
static constexpr uint8_t CMD_BASIC_INFO = 0x03;

void BmsClient::notifyCallback(BLERemoteCharacteristic *pChar, uint8_t *pData, size_t length, bool isNotify) {
    BmsClient::instance().decodeBmsData(pData, length);
}

BmsClient::BmsClient(const char *address, DataCallback dataCallback, StatusCallback statusCallback)
    : deviceAddress(address), dataCallback(dataCallback), statusCallback(statusCallback) {}

void BmsClient::setup() {
    Serial.println("BmsClient: Starting setup...");
    BLEDevice::init("");
    Serial.println("BmsClient: BLE initialized");
    statusCallback(ConnectionState::Connecting);
}

void BmsClient::update() {
    if (!isConnected()) {
        statusCallback(ConnectionState::Connecting);
        connectToServer();
    } else {
        requestBmsData();
    }
}

void BmsClient::decodeBmsData(uint8_t *data, size_t length) {
    if (length < 4 || data[0] != 0xDD) return;

    switch (data[1]) {
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
    Serial.println("BmsClient: Attempting to connect...");

    if (pClient != nullptr) {
        Serial.println("BmsClient: Cleaning up old client");
        delete pClient;
        pClient = nullptr;
    }

    pClient = BLEDevice::createClient();
    Serial.println("BmsClient: Client created");

    if (pClient->connect(BLEAddress(deviceAddress))) {
        Serial.println("BmsClient: Connected to server");

        pRemoteService = pClient->getService(SERVICE_UUID);
        if (pRemoteService == nullptr) {
            Serial.println("BmsClient: Failed to find service");
            statusCallback(ConnectionState::Connecting);
            return;
        }
        Serial.println("BmsClient: Found service");

        pNotifyChar = pRemoteService->getCharacteristic(CHAR_NOTIFY);
        pWriteChar = pRemoteService->getCharacteristic(CHAR_WRITE);

        if (pNotifyChar == nullptr || pWriteChar == nullptr) {
            Serial.println("BmsClient: Failed to find characteristics");
            statusCallback(ConnectionState::Connecting);
            return;
        }
        Serial.println("BmsClient: Found characteristics");

        pNotifyChar->registerForNotify(notifyCallback);
        Serial.println("BmsClient: Registered for notifications");
        statusCallback(ConnectionState::Connected);
    } else {
        Serial.println("BmsClient: Failed to connect");
        statusCallback(ConnectionState::Connecting);
    }
}