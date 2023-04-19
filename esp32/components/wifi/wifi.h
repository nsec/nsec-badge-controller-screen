#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

class Wifi
{
  public:
    static Wifi &getInstance()
    {
        static Wifi instance;
        return instance;
    }

  private:
    Wifi()
    {
    }

    static TaskHandle_t _taskHandle;

  public:
    Wifi(Wifi const &) = delete;
    void operator=(Wifi const &) = delete;

    void init();
    static void task(Wifi *instance) {
        instance->taskHandler();
    }
    void taskHandler();
};

#ifdef __cplusplus
}
#endif
