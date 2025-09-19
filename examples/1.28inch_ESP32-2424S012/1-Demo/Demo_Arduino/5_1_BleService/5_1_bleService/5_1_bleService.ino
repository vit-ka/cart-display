#include <BLEDevice.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setup() {
  BLEDevice::init("ESP32-BLE");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID); 
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                                  CHARACTERISTIC_UUID, 
                                                  BLECharacteristic::PROPERTY_READ | // 启用读取
                                                  BLECharacteristic::PROPERTY_WRITE  // 启用写入
                                                  );
  pCharacteristic->setValue("Hello World"); 
                                            
  pService->start(); 

  BLEDevice::startAdvertising();
}

void loop() {}
