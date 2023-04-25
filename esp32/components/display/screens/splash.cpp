#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include "screens/splash.h"

static const char *TAG = "display";

static lv_style_t style_box;
static lv_obj_t *label;
static lv_obj_t *progress;

#define SPLASH_STRINGS_LONG(str) str, str, str, str, str, str, str, str, str, str
#define SPLASH_STRINGS_SHORT(str) str, str, str

static const char *splash_strings[] = {
    "NorthSec 2023\nPersonal Assistant Device",
    SPLASH_STRINGS_LONG("Initializing"),
    SPLASH_STRINGS_LONG("Initializing."),
    SPLASH_STRINGS_LONG("Initializing.."),
    SPLASH_STRINGS_LONG("Initializing..."),
    SPLASH_STRINGS_LONG("Initializing...."),
    SPLASH_STRINGS_LONG(""),
    SPLASH_STRINGS_LONG("."),
    SPLASH_STRINGS_LONG(".."),
    SPLASH_STRINGS_LONG("..."),
    SPLASH_STRINGS_LONG(""),
    SPLASH_STRINGS_LONG("."),
    SPLASH_STRINGS_LONG(".."),
    SPLASH_STRINGS_LONG("..."),
    SPLASH_STRINGS_LONG(""),
    SPLASH_STRINGS_LONG("."),
    SPLASH_STRINGS_LONG(".."),
    SPLASH_STRINGS_LONG("..."),
    SPLASH_STRINGS_LONG(""),
    SPLASH_STRINGS_LONG("."),
    SPLASH_STRINGS_LONG(".."),
    SPLASH_STRINGS_LONG("..."),
    SPLASH_STRINGS_LONG(""),
    SPLASH_STRINGS_LONG("."),
    SPLASH_STRINGS_LONG(".."),
    SPLASH_STRINGS_LONG("..."),
    SPLASH_STRINGS_LONG("Loading assets..."),
    SPLASH_STRINGS_LONG("Loading strings..."),
    SPLASH_STRINGS_LONG("Configuring wifi..."),
    SPLASH_STRINGS_LONG("Configuring mesh network..."),
    SPLASH_STRINGS_LONG("Loading flags..."),
    SPLASH_STRINGS_SHORT("FLAG-{so_fast_84d45cceb0360957}"),
    SPLASH_STRINGS_LONG("Configuring dependencies..."),
};

void screen_splash_init()
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

    label = lv_label_create(cont, NULL);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

    progress = lv_bar_create(cont, NULL);
    lv_obj_set_size(progress, 200, 20);
    lv_obj_align(progress, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_anim_time(progress, 2000);
    lv_bar_set_range(progress, 0, sizeof(splash_strings) / sizeof(char *));

    screen_splash_set_string(0);
    return;
}

void screen_splash_set_string(int cur)
{
    if(cur >= screen_splash_string_count())
        return;

    lv_label_set_text(label, splash_strings[cur]);
    lv_bar_set_value(progress, cur, LV_ANIM_ON);
}

int screen_splash_string_count()
{
    return sizeof(splash_strings) / sizeof(char *);
}
