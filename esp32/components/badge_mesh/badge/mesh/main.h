#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#include "badge/mesh/network.h"

#ifdef __cplusplus
extern "C" {
#endif

class BadgeMesh
{
  public:
    static BadgeMesh &getInstance()
    {
        static BadgeMesh instance;
        return instance;
    }

  private:
    BadgeMesh()
    {
    }

    static TaskHandle_t _taskHandle;

  public:
    BadgeMesh(BadgeMesh const &) = delete;
    void operator=(BadgeMesh const &) = delete;

    void init();
    static void task(BadgeMesh *instance) {
        instance->taskHandler();
    }
    void taskHandler();
};

#ifdef __cplusplus
}
#endif
