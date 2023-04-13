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

    buf2 = heap_caps_malloc(size_in_px * sizeof(lv_color_t), MALLOC_CAP_8BIT);
    if (buf2 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buf2 %lu", size_in_px);
        return;
    }
    memset(buf2, 0, size_in_px);

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

void Display::taskHandler()
{
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

        xSemaphoreGive(xGuiSemaphore);
    }

    while(true) {
		if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelete(Display::_taskHandle);
}

void Display::demo()
{
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
	    lv_demo_widgets();
        xSemaphoreGive(xGuiSemaphore);
    }
}
