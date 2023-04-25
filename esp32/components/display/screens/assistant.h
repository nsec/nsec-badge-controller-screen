#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void screen_assistant_init();
void screen_assistant_loop();

#ifdef __cplusplus
}
#endif
