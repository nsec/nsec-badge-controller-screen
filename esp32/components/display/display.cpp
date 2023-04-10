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

static const char *TAG = "display";
#define LV_TICK_PERIOD_MS 10

TaskHandle_t Display::_taskHandle = NULL;

//Creates a semaphore to handle concurrent call to lvgl stuff
//If you wish to call *any* lvgl function from other threads/tasks
//you should lock on the very same semaphore!
SemaphoreHandle_t xGuiSemaphore;

void Display::init()
{
    xTaskCreatePinnedToCore((TaskFunction_t)&(Display::task), "display", 4096, this, 5, &Display::_taskHandle, 1);
}

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void Display::taskHandler()
{
    xGuiSemaphore = xSemaphoreCreateMutex();    // 创建GUI信号量
    lv_init();          // 初始化LittlevGL
    lvgl_driver_init(); // 初始化液晶SPI驱动 触摸芯片SPI/IIC驱动

    static lv_color_t buf1[DISP_BUF_SIZE];
#ifndef CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
    static lv_color_t buf2[DISP_BUF_SIZE];
#endif
    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

#if defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_IL3820
    /* Actual size in pixel, not bytes and use single buffer */
    size_in_px *= 8;
    lv_disp_buf_init(&disp_buf, buf1, NULL, size_in_px);
#elif defined CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
    lv_disp_buf_init(&disp_buf, buf1, NULL, size_in_px);
#else
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);
#endif

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

#ifdef CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
#endif

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

#if CONFIG_LVGL_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    // 一个标签演示
    //lv_obj_t * scr = lv_disp_get_scr_act(NULL);         // 获取当前屏幕
    //lv_obj_t * label1 =  lv_label_create(scr, NULL);    // 在当前活动的屏幕上创建标签
    //lv_label_set_text(label1, "Hello\nworld!");         // 修改标签的文字
    // 对象对齐函数，将标签中心对齐，NULL表示在父级上对齐，当前父级是屏幕，0，0表示对齐后的x，y偏移量
    //lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);
/*
	lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);
	lv_obj_t * cursor_obj =  lv_img_create(lv_scr_act(), NULL); //Create an image object for the cursor
	lv_img_set_src(cursor_obj, &mouse_cursor_icon);             //Set the image source
	lv_indev_set_cursor(mouse_indev, cursor_obj);               //Connect the image  object to the driver
*/
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
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
	    lv_demo_widgets();
        xSemaphoreGive(xGuiSemaphore);
    }
}
