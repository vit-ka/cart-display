#pragma once

#include <lvgl.h>
#include "display.hpp"
#include "common_types.hpp"

class DisplayManager {
public:
    void begin();
    void update(const BmsData& data);
    void handleTasks() { lv_task_handler(); }
    void updateConnectionState(ConnectionState state);

private:
    static constexpr int16_t POWER_BAR_DISCHARGING_MAX = 8000;  // 8kW
    static constexpr int16_t POWER_BAR_CHARGING_MAX = 4000;   // 4kW

    void setupLabels();
    void setupPowerBar();
    void updatePowerBar(const BmsData& data);
    void setupConnectionIcon();

    static lv_obj_t *voltage_label;
    static lv_obj_t *current_label;
    static lv_obj_t *power_label;
    static lv_obj_t *soc_label;
    static lv_obj_t *power_bar;
    static lv_obj_t *power_bar_label;
    static lv_obj_t *connection_icon;

    static void flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[2][240 * 10];
};