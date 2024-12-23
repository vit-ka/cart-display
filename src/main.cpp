#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>

#define BATTERY_NAME "Golfy"
#define BATTERY_ADDRESS "a4:c1:37:03:f9:fc"
#define SERVICE_UUID "0000ff00-0000-1000-8000-00805f9b34fb"
#define CHAR_NOTIFY "0000ff01-0000-1000-8000-00805f9b34fb"
#define CHAR_WRITE  "0000ff02-0000-1000-8000-00805f9b34fb"

static BLEClient* pClient = nullptr;
static boolean connected = false;
static BLERemoteService* pRemoteService;
static BLERemoteCharacteristic* pNotifyChar = nullptr;
static BLERemoteCharacteristic* pWriteChar = nullptr;

void decodeBmsData(uint8_t* data, size_t length) {
    if (length < 4) return;

    // Check packet header (0xDD)
    if (data[0] != 0xDD) return;

    // Decode based on command type
    switch(data[1]) {
        case 0x03: // Basic info
            if (length >= 13) {
                float voltage1 = (data[4] << 8 | data[5]) / 100.0;
                float current1 = ((int16_t)(data[6] << 8 | data[7])) / 100.0;
                uint8_t soc1 = data[10];

                Serial.println("=== Possible Decodings ===");
                Serial.printf("Voltage1: %.2fV (original)\n", voltage1);
                Serial.printf("Current1: %.2fA (original)\n", current1);
                Serial.printf("SOC1: %d%% (byte 10)\n", soc1);
            }
            break;
    }
}

static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Received: ");
    for(int i = 0; i < length; i++) {
        Serial.printf("%02X ", pData[i]);
    }
    Serial.println();

    decodeBmsData(pData, length);
}

void requestBmsData() {
    if (pWriteChar == nullptr) return;

    // Basic info request command
    uint8_t cmd[] = {0xDD, 0xA5, 0x03, 0x00, 0xFF, 0xFD, 0x77};
    pWriteChar->writeValue(cmd, sizeof(cmd));
}

void connectToServer() {
    if (pClient != nullptr) {
        delete pClient;
        pClient = nullptr;
    }

    Serial.println("Creating BLE client...");
    pClient = BLEDevice::createClient();

    Serial.printf("Attempting to connect to %s...\n", BATTERY_ADDRESS);
    if(pClient->connect(BLEAddress(BATTERY_ADDRESS))) {
        Serial.println("Connected!");
        connected = true;

        pRemoteService = pClient->getService(SERVICE_UUID);
        if(pRemoteService == nullptr) {
            Serial.println("Failed to find battery service");
            connected = false;
            return;
        }

        pNotifyChar = pRemoteService->getCharacteristic(CHAR_NOTIFY);
        pWriteChar = pRemoteService->getCharacteristic(CHAR_WRITE);

        if (pNotifyChar == nullptr || pWriteChar == nullptr) {
            Serial.println("Failed to find characteristics");
            connected = false;
            return;
        }

        pNotifyChar->registerForNotify(notifyCallback);
        Serial.println("Ready to communicate with BMS");
    } else {
        Serial.println("Connection failed - check if device is in range");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BAT-BMS Client...");
    BLEDevice::init("");
    connectToServer();
}

void loop() {
    if (!connected) {
        connectToServer();
        delay(1000);
    } else {
        requestBmsData();
        delay(2000);
    }
}