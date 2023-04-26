#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>
#include <esp_system.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "badge/mesh/config.h"
#include "badge/mesh/cmd/provision.h"

static const char *TAG = "cmd_provision";

static int provision_cmd(int argc, char **argv)
{
    uint16_t addr = 0;

    if (sscanf(argv[1], "%hu", &addr) != 1) {
        printf("address not valid (format is 0xffff)\n");
        return ESP_FAIL;
    }

    addr |= SCREEN_ADDRESS_RANGE;
    if(save_node_addr(addr) == ESP_OK) {
        printf("Saved node address 0x%04x for table %u\n", addr, addr % SCREEN_ADDRESS_RANGE);
    } else {
        printf("Node address not saved successfully... :(\n");
    }

    return ESP_OK;
}

void register_provision_commands(void)
{
    uint16_t addr;
    if(load_node_addr(&addr) == ESP_OK && addr > SCREEN_ADDRESS_RANGE && addr < (SCREEN_ADDRESS_RANGE + 0xff)) {
        ESP_LOGV(TAG, "Node address permanently set to 0x%04x", addr);
        return;
    }

    const esp_console_cmd_t cmd = {
        .command = "provision",
        .help = "Set unique ID for this device",
        .hint = NULL,
        .func = &provision_cmd,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    return;
}
