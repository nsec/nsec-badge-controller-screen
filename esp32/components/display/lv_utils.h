#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern lv_style_t style_box;
extern lv_style_t style_row_container; // no padding or margin

lv_obj_t *create_switch_with_label(lv_obj_t *parent, const char *text, bool enabled = false);
lv_obj_t *create_container(lv_obj_t *parent, const char *title = NULL, lv_layout_t layout = LV_LAYOUT_COLUMN_LEFT);

void util_styles_init();

#ifdef __cplusplus
}
#endif
