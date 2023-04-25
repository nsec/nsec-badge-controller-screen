#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void screen_splash_init();
void screen_splash_set_string(int cur);
int screen_splash_string_count();

#ifdef __cplusplus
}
#endif
