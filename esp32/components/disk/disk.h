#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Iteration stops when callback returns false */
typedef bool (* disk_iter_cb_t)(dirent *entry, void *param);

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
    sdmmc_card_t _card;
    bool _enabled;
    char _mount_point[10];

  public:
    Disk(Disk const &) = delete;
    void operator=(Disk const &) = delete;

    void init();
    static void task(Disk *instance) {
        instance->taskHandler();
    }
    void taskHandler();
    CardState getCardState() { return _cardState; }
    sdmmc_card_t *getCardInfo() {
        if(_cardState == CardState::Present)
            /* prone to race condition but this is statically allocated mem, should be fine? */
            return &_card;
        else
            return NULL;
    }

    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
    bool isEnabled() { return _enabled; }

    const char *getMountPoint() { return (char *)&_mount_point; }
    bool iterPath(const char *path, disk_iter_cb_t cb, void *param);
};

#ifdef __cplusplus
}
#endif
