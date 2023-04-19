#include <esp_system.h>
#include <esp_log.h>

#include "wifi.h"

static const char *TAG = "wifi";

TaskHandle_t Wifi::_taskHandle = NULL;

void Wifi::init()
{
    xTaskCreate((TaskFunction_t)&(Wifi::task), "wifi", 1024, this, 5, &Wifi::_taskHandle);
}

void Wifi::taskHandler()
{
    while(true) {

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(Wifi::_taskHandle);
}
