#include <Arduino.h>
#include <lvgl.h>
#include "display.hpp"
#include "bms_client.hpp"

LGFX tft;
static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][screenWidth * 10];
static lv_obj_t *voltage_label;
static lv_obj_t *current_label;
static lv_obj_t *power_label;
static lv_obj_t *soc_label;

#define BATTERY_ADDRESS "a4:c1:37:03:f9:fc"

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
    tft.fillScreen(TFT_BLACK);

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], screenWidth * 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Set theme to dark mode
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

    // Create battery status labels
    voltage_label = lv_label_create(lv_scr_act());
    current_label = lv_label_create(lv_scr_act());
    power_label = lv_label_create(lv_scr_act());
    soc_label = lv_label_create(lv_scr_act());

    // Set initial text
    lv_label_set_text(voltage_label, "Voltage: --.-V");
    lv_label_set_text(current_label, "Current: --.-A");
    lv_label_set_text(power_label, "Power: --.-W");
    lv_label_set_text(soc_label, "SOC: --%");

    // Make SOC label bigger
    lv_obj_set_style_text_font(soc_label, &lv_font_montserrat_20, 0);

    // Position labels in center
    lv_obj_align(voltage_label, LV_ALIGN_CENTER, 0, -60);
    lv_obj_align(current_label, LV_ALIGN_CENTER, 0, -20);
    lv_obj_align(power_label, LV_ALIGN_CENTER, 0, 20);
    lv_obj_align(soc_label, LV_ALIGN_CENTER, 0, 60);
}

void update_display(float voltage, float current, float power, int soc) {
    static char buf[32];

    snprintf(buf, sizeof(buf), "Voltage: %.1fV", voltage);
    lv_label_set_text(voltage_label, buf);

    snprintf(buf, sizeof(buf), "Current: %.1fA", current);
    lv_label_set_text(current_label, buf);

    if (abs(power) >= 1000) {
        snprintf(buf, sizeof(buf), "Power: %.2fkW", power/1000);
    } else {
        snprintf(buf, sizeof(buf), "Power: %.1fW", power);
    }
    lv_label_set_text(power_label, buf);

    snprintf(buf, sizeof(buf), "SOC: %d%%", soc);
    lv_label_set_text(soc_label, buf);

    // Set SOC color based on level
    if (soc <= 15) {
        lv_obj_set_style_text_color(soc_label, lv_color_make(255, 0, 0), 0);  // Red
    } else if (soc >= 80) {
        lv_obj_set_style_text_color(soc_label, lv_color_make(0, 255, 0), 0);  // Green
    } else {
        lv_obj_set_style_text_color(soc_label, lv_color_white(), 0);  // White
    }
}

void onBmsData(const BmsClient::BmsData& data) {
    update_display(data.voltage, data.current, data.power, data.soc);
}

BmsClient bms(BATTERY_ADDRESS, onBmsData);

void setup() {
    Serial.begin(115200);
    setup_display();
    bms.begin();
}

void loop() {
    bms.update();
    lv_task_handler();
    delay(100);
}