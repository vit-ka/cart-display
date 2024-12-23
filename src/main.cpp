#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEClient.h>
#include <lvgl.h>
#include "display.hpp"

LGFX tft;
static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][screenWidth * 10];

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

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    if (tft.getStartCount() == 0) {
        tft.endWrite();
    }
    tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1,
                     (lgfx::swap565_t *)&color_p->full);
    lv_disp_flush_ready(disp);
}

void setup_display() {
    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);

    tft.init();
    tft.initDMA();
    tft.startWrite();

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Test label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void update_display(float voltage, float current, float power, int soc) {
    static char buf[32];

    // snprintf(buf, sizeof(buf), "Voltage: %.1fV", voltage);
    // lv_label_set_text(voltage_label, buf);

    // snprintf(buf, sizeof(buf), "Current: %.1fA", current);
    // lv_label_set_text(current_label, buf);

    // snprintf(buf, sizeof(buf), "Power: %.1fW", power);
    // lv_label_set_text(power_label, buf);

    // snprintf(buf, sizeof(buf), "SOC: %d%%", soc);
    // lv_label_set_text(soc_label, buf);
}

void decodeBmsData(uint8_t* data, size_t length) {
    if (length < 4 || data[0] != 0xDD) return;

    switch(data[1]) {
        case 0x03:
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
                update_display(voltage, current, power, soc);
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

    uint8_t cmd[7] = {0xDD, 0xA5, CMD_BASIC_INFO, 0x00, 0xFF, 0xFD, 0x77};
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
    setup_display();
    BLEDevice::init("");
    connectToServer();
}

void loop() {
    if (!connected) {
        connectToServer();
        delay(1000);
    } else {
        requestBmsData();
        lv_task_handler();
        delay(100);
    }
}