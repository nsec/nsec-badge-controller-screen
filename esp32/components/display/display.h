#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#ifdef __cplusplus
extern "C" {
#endif

class Display
{
  public:
    static Display &getInstance()
    {
        static Display instance;
        return instance;
    }

  private:
    Display()
    {
    }

    static TaskHandle_t _taskHandle;

  public:
    Display(Display const &) = delete;
    void operator=(Display const &) = delete;

    void init();
    void demo();
    static void task(Display *instance) {
        instance->taskHandler();
    }
    void taskHandler();
};

#ifdef __cplusplus
}
#endif
