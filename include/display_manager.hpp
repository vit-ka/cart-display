#pragma once

#include <lvgl.h>
#include "display.hpp"

class DisplayManager {
public:
    void begin();
    void update(float voltage, float current, float power, int soc);
    void handleTasks() { lv_task_handler(); }

private:
    void setupLabels();

    static lv_obj_t *voltage_label;
    static lv_obj_t *current_label;
    static lv_obj_t *power_label;
    static lv_obj_t *soc_label;

    static void flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[2][240 * 10];
};