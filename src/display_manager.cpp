#include "display_manager.hpp"

lv_obj_t* DisplayManager::voltage_label = nullptr;
lv_obj_t* DisplayManager::current_label = nullptr;
lv_obj_t* DisplayManager::power_label = nullptr;
lv_obj_t* DisplayManager::soc_label = nullptr;
lv_disp_draw_buf_t DisplayManager::draw_buf;
lv_color_t DisplayManager::buf[2][240 * 10];
lv_obj_t* DisplayManager::power_bar = nullptr;
lv_obj_t* DisplayManager::power_bar_label = nullptr;
lv_obj_t* DisplayManager::connection_icon = nullptr;

extern LGFX tft;

void DisplayManager::flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    if (tft.getStartCount() == 0) {
        tft.endWrite();
    }
    tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1,
                     (lgfx::swap565_t *)&color_p->full);
    lv_disp_flush_ready(disp);
}

void DisplayManager::begin() {
    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);

    tft.init();
    tft.initDMA();
    tft.startWrite();
    tft.fillScreen(TFT_BLACK);

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], 240 * 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = flushDisplay;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);

    setupLabels();
    setupConnectionIcon();

    // Force initial display update
    lv_task_handler();
}

void DisplayManager::setupLabels() {
    voltage_label = lv_label_create(lv_scr_act());
    current_label = lv_label_create(lv_scr_act());
    power_label = lv_label_create(lv_scr_act());
    soc_label = lv_label_create(lv_scr_act());

    lv_label_set_text(voltage_label, "Voltage: --.-V");
    lv_label_set_text(current_label, "Current: --.-A");
    lv_label_set_text(soc_label, "SOC: --%");

    lv_obj_set_style_text_font(soc_label, &lv_font_montserrat_20, 0);

    lv_obj_align(voltage_label, LV_ALIGN_CENTER, 0, -80);
    lv_obj_align(current_label, LV_ALIGN_CENTER, 0, -40);
    setupPowerBar();
    lv_obj_align(soc_label, LV_ALIGN_CENTER, 0, 55);
}

void DisplayManager::setupPowerBar() {
    // Create container for power bar and label
    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 220, 40);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cont, 0, 0);

    // Create power bar
    power_bar = lv_bar_create(cont);
    lv_obj_set_size(power_bar, 220, 40);
    lv_obj_align(power_bar, LV_ALIGN_CENTER, 0, 00);
    lv_bar_set_value(power_bar, 0, LV_ANIM_ON);


    // Style for background
    lv_obj_set_style_bg_color(power_bar, lv_color_make(40, 40, 40), LV_PART_MAIN);

    // Style for indicator
    lv_obj_set_style_bg_color(power_bar, lv_color_make(0, 150, 0), LV_PART_INDICATOR);
    lv_obj_set_style_anim_time(power_bar, 400, LV_PART_INDICATOR);  // Animation duration

    // Create label that matches bar size
    power_bar_label = lv_label_create(cont);
    lv_obj_set_style_text_font(power_bar_label, &lv_font_montserrat_20, 0);
    lv_obj_set_size(power_bar_label, 220, 40);
    lv_obj_set_style_text_align(power_bar_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(power_bar_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_ver(power_bar_label, 8, 0);
    lv_label_set_text(power_bar_label, "0 W");
}

void DisplayManager::updatePowerBar(float power) {
    lv_color_t barColor, barTextColor;
    // Adjust range and value based on charging/discharging.
    if (power > 0) {
        // Charging mode
        lv_bar_set_range(power_bar, 0, POWER_BAR_CHARGING_MAX);
        lv_bar_set_value(power_bar, power, LV_ANIM_ON);
        barColor = lv_color_make(0, 150, 0);      // Green for charging
        barTextColor = lv_color_white();
    } else if (power < 0) {
        // Discharging mode
        lv_bar_set_range(power_bar, 0, POWER_BAR_DISCHARGING_MAX);
        lv_bar_set_value(power_bar, -power, LV_ANIM_ON);
        barColor = lv_color_make(150, 0, 0);      // Red for discharging
        barTextColor = lv_color_white();
    } else {
        // Zero power: show minimal bar
        lv_bar_set_range(power_bar, 0, 0);
        lv_bar_set_value(power_bar, power, LV_ANIM_ON);
        barColor = lv_color_make(100, 100, 100);  // Grey for no power
        barTextColor = lv_color_make(100,100,100);
    }

    lv_obj_set_style_bg_color(power_bar, barColor, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(power_bar_label, barTextColor, 0);

    // Update text
    static char buf[32];
    if (abs(power) >= 1000) {
        snprintf(buf, sizeof(buf), "%.2f kW", power/1000);
    } else {
        snprintf(buf, sizeof(buf), "%.0f W", power);
    }
    lv_label_set_text(power_bar_label, buf);
}

void DisplayManager::update(float voltage, float current, float power, int soc) {
    static char buf[32];

    snprintf(buf, sizeof(buf), "Voltage: %.1fV", voltage);
    lv_label_set_text(voltage_label, buf);

    snprintf(buf, sizeof(buf), "Current: %.1fA", current);
    lv_label_set_text(current_label, buf);

    updatePowerBar(power);

    snprintf(buf, sizeof(buf), "SOC: %d%%", soc);
    lv_label_set_text(soc_label, buf);

    if (soc <= 15) {
        lv_obj_set_style_text_color(soc_label, lv_color_make(255, 0, 0), 0);
    } else if (soc >= 80) {
        lv_obj_set_style_text_color(soc_label, lv_color_make(0, 255, 0), 0);
    } else {
        lv_obj_set_style_text_color(soc_label, lv_color_white(), 0);
    }
}

void DisplayManager::setupConnectionIcon() {
    connection_icon = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(connection_icon, &lv_font_montserrat_14, 0);
    lv_obj_set_size(connection_icon, 220, 30);
    lv_obj_set_style_text_align(connection_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(connection_icon, LV_ALIGN_CENTER, 0, 100);
    lv_label_set_text(connection_icon, "Waiting...");
    lv_obj_set_style_text_color(connection_icon, lv_color_make(100, 100, 100), 0);  // Gray for waiting
}

void DisplayManager::updateConnectionState(ConnectionState state) {
    switch (state) {
        case ConnectionState::Connecting:
            lv_label_set_text(connection_icon, "Connecting...");
            lv_obj_set_style_text_color(connection_icon, lv_color_make(255, 255, 0), 0);  // Bright yellow
            break;
        case ConnectionState::Connected:
            lv_label_set_text(connection_icon, "Connected");
            lv_obj_set_style_text_color(connection_icon, lv_color_make(0, 255, 0), 0);  // Bright green
            break;
    }
}