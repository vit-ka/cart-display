#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>

#define BATTERY_NAME "Golfy"
#define BATTERY_ADDRESS "a4:c1:37:03:f9:fc"
#define SERVICE_UUID "0000ff00-0000-1000-8000-00805f9b34fb"
#define CHAR_NOTIFY "0000ff01-0000-1000-8000-00805f9b34fb"
#define CHAR_WRITE  "0000ff02-0000-1000-8000-00805f9b34fb"
#define CMD_BASIC_INFO 0x03
#define CMD_SOC_INFO  0x05

static BLEClient* pClient = nullptr;
static boolean connected = false;
static BLERemoteService* pRemoteService;
static BLERemoteCharacteristic* pNotifyChar = nullptr;
static BLERemoteCharacteristic* pWriteChar = nullptr;

void decodeBmsData(uint8_t* data, size_t length) {
    if (length < 4 || data[0] != 0xDD) return;

    switch(data[1]) {
        case 0x03: // Basic info
            if (length >= 13) {
                float voltage = (data[4] << 8 | data[5]) / 100.0;
                float current = ((int16_t)(data[6] << 8 | data[7])) / 100.0;
                uint16_t soc = (data[8] << 8 | data[9]) / 100;
                float power = voltage * current;

                Serial.println("\n=== Battery Status ===");
                Serial.printf("Voltage: %.1fV\n", voltage);
                Serial.printf("Current: %.1fA\n", current);
                Serial.printf("Power: %.1fW\n", power);
                Serial.printf("SOC: %d%%\n", soc);
                Serial.println("===================");
            }
            break;

        case 0x05: // SOC details
            if (length >= 8) {
                uint16_t fullCapacity = data[4] << 8 | data[5];
                uint16_t remainingCapacity = data[6] << 8 | data[7];
                float capacityPercent = (remainingCapacity * 100.0) / fullCapacity;

                Serial.println("\n=== Capacity Info ===");
                Serial.printf("Full Capacity: %dmAh\n", fullCapacity);
                Serial.printf("Remaining: %dmAh\n", remainingCapacity);
                Serial.printf("Percentage: %.1f%%\n", capacityPercent);
                Serial.println("==================");
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

    static uint8_t currentCmd = CMD_BASIC_INFO;
    uint8_t cmd[7] = {0xDD, 0xA5, 0x00, 0x00, 0xFF, 0x00, 0x77};

    switch(currentCmd) {
        case CMD_BASIC_INFO:
            cmd[2] = CMD_BASIC_INFO;
            cmd[5] = 0xFD;
            currentCmd = CMD_SOC_INFO;
            break;

        case CMD_SOC_INFO:
            cmd[2] = CMD_SOC_INFO;
            cmd[5] = 0xFF;
            currentCmd = CMD_BASIC_INFO;
            break;
    }

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