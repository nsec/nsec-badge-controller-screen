#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include <stdint.h>

#include "lvgl/lvgl.h"

#include "screens/debug.h"

struct SaveData {
    uint8_t neopixel_brightness;
    uint8_t neopixel_mode;
    bool neopixel_is_on;
    int neopixel_color;
    bool debug_enabled;
    uint16_t debug_pin;

    bool debug_feature_enabled[debug_tab::count];
    uint8_t mood_brightness;
    uint8_t mood_mode;
    lv_color_t mood_color;
};

class Save
{
  public:
    static esp_err_t save_log_level(const char *log, esp_log_level_t level);
    static esp_err_t load_and_set_log_levels();
    static esp_err_t clear_log_levels();
    static esp_err_t write_save();
    static esp_err_t load_save();
    static SaveData save_data;

};
