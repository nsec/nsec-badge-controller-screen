#include "cmd_modbus.h"

#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_log.h"
#include "modbus.h"

static const char *TAG = "SLAVE_TEST";

/** 'mb_read' command restarts the program */

static int mb_start(int argc, char **argv)
{
    xTaskCreate(modbus_task, "modbus_task", 4096, NULL, 4, NULL);
    ESP_LOGI(TAG, "Modbus read");
    return 0;
}

void register_mb_start(void)
{
    const esp_console_cmd_t cmd = {
        .command = "mb_start",
        .help = "Start Modbus",
        .hint = NULL,
        .func = &mb_start,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int mb_stop(int argc, char **argv)
{
    mobdbus_stop();
    ESP_LOGI(TAG, "Modbus stopped");
    return 0;
}

void register_mb_stop(void)
{
    const esp_console_cmd_t cmd = {
        .command = "mb_stop",
        .help = "Stop Modbus",
        .hint = NULL,
        .func = &mb_stop,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}