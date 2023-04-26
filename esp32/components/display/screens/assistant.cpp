#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "badge/mesh/main.h"
#include "screens/assistant.h"

static const char *TAG = "display";

static lv_style_t style_box;
static lv_obj_t *time_label;

void screen_assistant_init()
{
    lv_style_init(&style_box);
    lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, - LV_DPX(10));
    lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(30));

    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(cont, true);                    /*Auto realign when the size changes*/
    lv_obj_align_origo(cont, NULL, LV_ALIGN_CENTER, 0, 0);  /*This parametrs will be sued when realigned*/
    lv_cont_set_fit(cont, LV_FIT_MAX);
    lv_cont_set_layout(cont, LV_LAYOUT_CENTER);

    time_label = lv_label_create(cont, NULL);
    lv_label_set_align(time_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(time_label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_font(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_48);
    lv_label_set_text(time_label, "--:--");

    return;
}

void screen_assistant_loop()
{
    struct tm tm;
    time_t t;
    char buf[32];

    if(BadgeMesh::getInstance().networkTimeIsValid()) {
        BadgeMesh::getInstance().networkTimeGet(&t);
        gmtime_r(&t, &tm);
        snprintf((char *)&buf, sizeof(buf), "%02d:%02d", tm.tm_hour, tm.tm_min);
        lv_label_set_text(time_label, buf);
    }

    return;
}
