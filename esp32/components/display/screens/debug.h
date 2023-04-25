#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void screen_debug_init();
void screen_debug_loop();

#ifdef __cplusplus
}
#endif
