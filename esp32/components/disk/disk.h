#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

class Disk
{
  public:
    static Disk &getInstance()
    {
        static Disk instance;
        return instance;
    }

    enum CardState {
        NotPresent, // no card inserted, or not yet initialized
        NotReadable, // filesystem cannot be mounted
        Present, // card is inserted and has been mounted
        Failed, // card used to be present but needs to be unmounted
    };

  private:
    Disk()
    {
    }

    static TaskHandle_t _taskHandle;
    CardState _cardState;

  public:
    Disk(Disk const &) = delete;
    void operator=(Disk const &) = delete;

    void init();
    static void task(Disk *instance) {
        instance->taskHandler();
    }
    void taskHandler();
    CardState getCardState() { return _cardState; }
};

#ifdef __cplusplus
}
#endif
