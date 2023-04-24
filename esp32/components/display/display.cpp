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
#include "lvgl/lvgl.h"			// LVGL头文件
#include "lvgl_helpers.h"		// 助手 硬件驱动相关

#include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"

#include "display.h"
#include "buzzer.h"

static const char *TAG = "display";
#define LV_TICK_PERIOD_MS 10

#define SPLASH_WAIT_SECONDS 3

TaskHandle_t Display::_taskHandle = NULL;

//Creates a semaphore to handle concurrent call to lvgl stuff
//If you wish to call *any* lvgl function from other threads/tasks
//you should lock on the very same semaphore!
SemaphoreHandle_t xGuiSemaphore;

static lv_disp_buf_t _lv_display_buffer;

static void _initialize_lv_buffers()
{
    static void *buf1 = NULL;
    static void *buf2 = NULL;
    uint32_t size_in_px = DISP_BUF_SIZE;

    lv_init();
    lvgl_driver_init();

    buf1 = heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_8BIT);
    if (buf1 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buf1 %lu", size_in_px);
        return;
    }
    memset(buf1, 0, size_in_px);

    /*buf2 = heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_8BIT);
    if (buf2 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buf2 %lu", size_in_px);
        return;
    }
    memset(buf2, 0, size_in_px);*/

    lv_disp_buf_init(&_lv_display_buffer, buf1, buf2, size_in_px);

}

void Display::init()
{
    xGuiSemaphore = xSemaphoreCreateMutex();

    _initialize_lv_buffers();

    xTaskCreatePinnedToCore((TaskFunction_t)&(Display::task), "display", 4096, this, 5, &Display::_taskHandle, 1);

    // wait for task to start running
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}


/** Called when an action happened on the input device.
 * The second parameter is the event from `lv_event_t`*/
static void touch_feedback(struct _lv_indev_drv_t *, lv_event_t evt)
{
    switch(evt) {
        case LV_EVENT_CLICKED: {
            Buzzer::getInstance().buzz(100, 50);
            break;
        }
    }
}

void Display::showMessage(char *msg)
{
	if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
        static const char * btns[] = {"Close", ""};
        lv_obj_t * m = lv_msgbox_create(lv_scr_act(), NULL);
        lv_msgbox_set_text(m, msg);
        lv_msgbox_add_btns(m, btns);
        lv_obj_t * btnm = lv_msgbox_get_btnmatrix(m);
        lv_btnmatrix_set_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECK_STATE);

        Buzzer::getInstance().play(Buzzer::Sounds::ImportantNotice);

        xSemaphoreGive(xGuiSemaphore);
    }

    return;
}

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
    SPLASH_STRINGS_SHORT("FLAG-{so_fast_c571e25978f76ee}"),
    SPLASH_STRINGS_LONG("Configuring dependencies..."),
};

static lv_style_t splash_style_box;
static lv_obj_t *splash_label;
static lv_obj_t *splash_progress;

void splash_init(int str)
{
    // lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    // lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_style_init(&splash_style_box);
    lv_style_set_value_align(&splash_style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&splash_style_box, LV_STATE_DEFAULT, - LV_DPX(10));
    lv_style_set_margin_top(&splash_style_box, LV_STATE_DEFAULT, LV_DPX(30));

    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    // lv_cont_set_layout(container, LV_LAYOUT_PRETTY_MID);
    // lv_obj_add_style(container, LV_CONT_PART_MAIN, &splash_style_box);

    // lv_cont_set_fit2(container, LV_FIT_NONE, LV_FIT_TIGHT);
    // lv_obj_set_width(container, grid_w);

    lv_obj_set_auto_realign(cont, true);                    /*Auto realign when the size changes*/
    lv_obj_align_origo(cont, NULL, LV_ALIGN_CENTER, 0, 0);  /*This parametrs will be sued when realigned*/
    lv_cont_set_fit(cont, LV_FIT_MAX);
    lv_cont_set_layout(cont, LV_LAYOUT_CENTER);

    splash_label = lv_label_create(cont, NULL);
    lv_label_set_align(splash_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(splash_label, NULL, LV_ALIGN_CENTER, 0, 0);

    splash_progress = lv_bar_create(cont, NULL);
    lv_obj_set_size(splash_progress, 200, 20);
    lv_obj_align(splash_progress, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_anim_time(splash_progress, 2000);
    lv_bar_set_range(splash_progress, 0, sizeof(splash_strings) / sizeof(char *));
}

void splash_update(int str)
{
    lv_label_set_text(splash_label, splash_strings[str]);
    lv_bar_set_value(splash_progress, str, LV_ANIM_ON);
}

void Display::taskHandler()
{
    TickType_t splash_displayed_at = xTaskGetTickCount();
    bool splash_visible = true;
    int splash_cur = 0;

	if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
        lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);
        disp_drv.flush_cb = disp_driver_flush;
        disp_drv.buffer = &_lv_display_buffer;
        lv_disp_drv_register(&disp_drv);

        lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.read_cb = touch_driver_read;
        indev_drv.feedback_cb = touch_feedback;
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        lv_indev_drv_register(&indev_drv);

        const esp_timer_create_args_t periodic_timer_args = {
            .callback = &lv_tick_task,
            .arg = NULL,
            .name = "periodic_gui",
        };
        esp_timer_handle_t periodic_timer;
        ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

        lv_obj_clean(lv_scr_act());
        splash_init(0);
        splash_update(0);

        xSemaphoreGive(xGuiSemaphore);
    }

    while(true) {
		if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
            if(splash_visible) {
                TickType_t elapsed_ticks = xTaskGetTickCount() - splash_displayed_at;
                uint32_t elapsed_time_ms = (uint32_t)((elapsed_ticks * 1000) / configTICK_RATE_HZ);

                if(elapsed_time_ms > (SPLASH_WAIT_SECONDS * 1000)) {
                    if(splash_cur > sizeof(splash_strings) / sizeof(char *)) {
                        lv_demo_widgets();
                        splash_visible = false;
                    } else {
                        splash_update(splash_cur++);
                    }
                }
            }

            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelete(Display::_taskHandle);
}
