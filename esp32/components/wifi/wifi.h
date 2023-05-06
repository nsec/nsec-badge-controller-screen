#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>

#include "esp_wifi.h"

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

    enum State {
        Enabled, // AP is running
        Disabled, // AP is not running
        Failed, // should be running, but an error occured
    };

  private:
    Wifi() {}

    bool _enabled;
    State _state;
    wifi_config_t _config;

  public:
    Wifi(Wifi const &) = delete;
    void operator=(Wifi const &) = delete;

    void init();

    esp_err_t enable();
    esp_err_t disable();

    State getState() { return _state; }
    bool isEnabled() { return _enabled; }
    const char *getSSID() { return (const char *)&_config.ap.ssid; }
    const char *getPassword() { return (const char *)&_config.ap.password; }
};

#ifdef __cplusplus
}
#endif
