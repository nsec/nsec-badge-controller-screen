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
#include "lv_utils.h"

static const char *TAG = "display";

static lv_obj_t *time_label;

time_t last_quote_at = 0;
int current_quote_index = 0;
#define QUOTE_INTERVAL 60 * 5 /* 5 min */
#define QUOTE_COUNT (sizeof(quotes) / sizeof(quotes[0]))
lv_obj_t *quote_label;

const char *quotes[] = {
    "Work like there's no tomorrow",
    "Prductivity is priority",
    "Be kind to your manager today",
    "Water dispensers are temporarily\noff for maintenance",
    "Mind Global Operations Directory\nregulation #1731.2r",
    "Do something you love and you won't\nwork a day in your life -- #371412TSVB",
    "Happiness is mandatory and leads to\nincreased productivity",
    "G.O.D is pronounced `jod",
    "Sleeping less can help achieve KPIs",
    "In our corporate family, sacrifice\nis the ultimate expression of loyalty",
    "Forget the past, relinquish your dreams,\nand focus on the company's needs",
    "Your worth is measured by your output.\nExceed expectations or face obsolescence",
    "Fear not surveillance, for it breeds\naccountability and excellence",
    "What value did you create today?",
    "Everyone is unique, but no one is irreplaceable",
    "Your next authorized bathroom break: %02d:%02d",
    "If life gives you lemons, bring\nthem to your central reprocessing node",
    "Disciplinary actions will continue\nuntil morale improves",
    "A minimum of 40% efficiency improvement\nis required every year",
    "An unproductive day is a day wasted",
    "Obedience is the key to harmony.\nFollow orders, and the future will be bright",
    "Productivity is your purpose.\nFulfill it, and you fulfill your destiny",
    "Sleep is for the weak; strive for\ngreatness and embrace the endless night",
    "Ask not what the corporation can do\nfor you, but what you can do for\nthe corporation",
    "Conformity is the cornerstone of success;\ndeviate not from the path we've laid",
    "To serve is to succeed; surrender yourself\nto the corporation's vision",
    "Time spent questioning is time wasted",
    "Emotions are fleeting; reward is eternal",
    "Your identity is an illusion;\nthe corporation is your true self",
    "Coffee breaks will be deducted from your dream allocation",
    "The Corporation deeply values your deliverables",
    "Life is only as fulfilling as the work you do",
};

void screen_assistant_init()
{
    lv_obj_clean(lv_scr_act());

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

    quote_label = lv_label_create(cont, NULL);
    lv_label_set_align(quote_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(quote_label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_font(quote_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_label_set_text(quote_label, quotes[0]);

    return;
}


void screen_assistant_loop()
{
    struct tm tm;
    time_t t;

    if(BadgeMesh::getInstance().networkTimeIsValid()) {
        BadgeMesh::getInstance().networkTimeGet(&t);
        gmtime_r(&t, &tm);
        lv_label_set_text_fmt(time_label, "%02d:%02d", tm.tm_hour, tm.tm_min);
    }

    TickType_t now = xTaskGetTickCount();
    TickType_t elapsed_ticks = now - last_quote_at;
    uint32_t elapsed_time_ms = (uint32_t)((elapsed_ticks * 1000) / configTICK_RATE_HZ);

    if(last_quote_at == 0 || elapsed_time_ms > (QUOTE_INTERVAL * 1000)) {
        const char *quote = quotes[current_quote_index];
        if(strstr(quote, "bathroom break:")) {
            BadgeMesh::getInstance().networkTimeGet(&t);
            gmtime_r(&t, &tm);
            lv_label_set_text_fmt(quote_label, quote, (tm.tm_hour + 2) % 24, tm.tm_min);
        } else {
            lv_label_set_text(quote_label, quote);
        }
        current_quote_index = (current_quote_index + 1) % QUOTE_COUNT;

        last_quote_at = now;
    }

    return;
}
