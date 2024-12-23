#pragma once

#include <lvgl.h>
#include "display.hpp"

class DisplayManager {
public:
    void begin();
    void update(float voltage, float current, float power, int soc);
    void handleTasks() { lv_task_handler(); }

private:
    static constexpr int16_t POWER_BAR_MIN = -4000;  // -4kW
    static constexpr int16_t POWER_BAR_MAX = 4000;   // +4kW

    void setupLabels();
    void setupPowerBar();
    void updatePowerBar(float power);

    static lv_obj_t *voltage_label;
    static lv_obj_t *current_label;
    static lv_obj_t *power_label;
    static lv_obj_t *soc_label;
    static lv_obj_t *power_bar;
    static lv_obj_t *power_bar_label;

    static void flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[2][240 * 10];
};