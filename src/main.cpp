#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>

#define BATTERY_NAME "Golfy"
#define BATTERY_ADDRESS "a4:c1:37:03:f9:fc"
#define SERVICE_UUID "0000ff00-0000-1000-8000-00805f9b34fb"

static BLEClient* pClient = nullptr;
static boolean connected = false;
static BLERemoteService* pRemoteService;

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
    }
  } else {
    Serial.println("Connection failed - check if device is in range");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client...");
  BLEDevice::init("");
  connectToServer();
}

void loop() {
  if (!connected) {
    connectToServer();
    delay(1000);
  } else {
    Serial.println("Connected to Golfy - reading data...");
    delay(2000);
  }
}