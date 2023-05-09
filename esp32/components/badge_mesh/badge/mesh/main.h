#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include <stdint.h>
#include <stdio.h>

#include "badge/mesh/network.h"
#include "badge/mesh/config.h"

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

    enum State {
        Enabled, // mesh is running
        Disabled, // mesh is not running
        Failed, // should be running, but an error occured
    };


  private:
    BadgeMesh()
    {
    }

    // static TaskHandle_t _taskHandle;
    SemaphoreHandle_t _bt_semaphore;

    bool networkTimeValid;
    bool _enabled;
    State _state;

  public:
    BadgeMesh(BadgeMesh const &) = delete;
    void operator=(BadgeMesh const &) = delete;

    void init();
    void taskHandler();
    esp_err_t enable();
    esp_err_t disable();

    bool getState() { return _state; }
    bool isEnabled() { return _enabled; }

    esp_err_t clientSend(uint16_t dst_addr, uint32_t op, uint8_t *msg, unsigned int length, bool needsResponse = false, uint8_t ttl = DEFAULT_TTL);
    esp_err_t serverSend(uint16_t dst_addr, uint32_t op, uint8_t *msg, unsigned int length);

    bool networkTimeIsValid();
    esp_err_t networkTimeSet(time_t now);
    esp_err_t networkTimeGet(time_t *now);
    esp_err_t networkTimeRequest();
};

esp_err_t mesh_client_send(uint16_t dst_addr, uint32_t op, uint8_t *msg, unsigned int length, bool needsResponse);
esp_err_t mesh_server_send(uint16_t dst_addr, uint32_t op, uint8_t *msg, unsigned int length);

#ifdef __cplusplus
}
#endif
