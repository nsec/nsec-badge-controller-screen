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
#include "driver/uart.h"

#include "console.h"
#include "badge/mesh/main.h"
#include "badge/mesh/config.h"
#include "badge/mesh/cmd/admin.h"
#include "badge/mesh/network.h"
#include "badge/mesh/host.h"
#include "badge/mesh/config.h"
#include "badge/mesh/ops/set_name.h"
#include "badge/mesh/ops/census.h"
#include "badge/mesh/ops/ping.h"
#include "badge/mesh/ops/ui_message.h"
#include "badge/mesh/ops/info.h"
#include "badge/mesh/ops/neopixel.h"
#include "badge/mesh/ops/time.h"

#if CONFIG_BADGE_MESH_ADMIN_COMMANDS

static const char *TAG = "badge/mesh";
#define MESH_ADMIN_CMD "mesh-admin"

static int show_cmd(int argc, char **argv);
static int help_cmd(int argc, char **argv);
static int census_cmd(int argc, char **argv);
static int ping_cmd(int argc, char **argv);
static int set_name_cmd(int argc, char **argv);
static int ui_message_cmd(int argc, char **argv);
static int info_cmd(int argc, char **argv);
static int neopixel_cmd(int argc, char **argv);
static int time_cmd(int argc, char **argv);
static int flag_request_cmd(int argc, char **argv);

static struct {
    struct arg_str *address;
    struct arg_str *name;
    struct arg_end *end;
} set_name_args;

static struct {
    struct arg_int *address;
    struct arg_end *end;
} flag_request_args;

static struct {
    struct arg_int *time;
    struct arg_int *mode;
    struct arg_int *brightness;
    struct arg_int *color;
    struct arg_int *red;
    struct arg_int *green;
    struct arg_int *blue;
    struct arg_int *ttl;
    struct arg_lit *all_modes;
    struct arg_end *end;
} neopixel_args;

static struct {
    struct arg_int *now;
    struct arg_end *end;
} time_args;

/*
    Kind of a hack to get sub-enus instead of all commands at the top level.
*/
const esp_console_cmd_t subcommands[] = {
    {
        .command = "help",
        .help = "Print the list of registered commands",
        .hint = NULL,
        .func = &help_cmd,
    },
    {
        .command = "show",
        .help = "Print information about the mesh",
        .hint = NULL,
        .func = &show_cmd,
    },
    {
        .command = "census",
        .help = "Start or stop a network-wide census of all nodes",
        .hint = NULL,
        .func = &census_cmd,
    },
    {
        .command = "ping",
        .help = "Ping a node",
        .hint = NULL,
        .func = &ping_cmd,
    },
    {
        .command = "info",
        .help = "Send an information request to a node",
        .hint = NULL,
        .func = &info_cmd,
    },
    {
        .command = "set-name",
        .help = "Set a new name for a node in the mesh",
        .hint = NULL,
        .func = &set_name_cmd,
        .argtable = &set_name_args,
    },
    {
        .command = "ui-message",
        .help = "Send a message to a device and display it on the screen",
        .hint = NULL,
        .func = &ui_message_cmd,
    },
    {
        .command = "neopixel",
        .help = "Send high-priority neopixel command to nearby badges",
        .hint = NULL,
        .func = &neopixel_cmd,
        .argtable = &neopixel_args,
    },
    {
        .command = "set-time",
        .help = "Set network time for this device and propagate to all other devices",
        .hint = NULL,
        .func = &time_cmd,
        .argtable = &time_args,
    },
    {
        .command = "test-flag-request",
        .help = "Send flag request to a node, just for testing that the command works",
        .hint = NULL,
        .func = &flag_request_cmd,
        .argtable = &flag_request_args,
    },
};

static int show_cmd(int argc, char **argv)
{
    printf("Mesh:\n");
    printf("  → host initialized: %s\n", mesh_host_initialized ? "yes" : "no");
    printf("  → provisioned: %s\n", mesh_is_provisioned() ? "yes" : "no");
    printf("Node:\n");
    printf("  → address: 0x%04x\n", badge_network_info.unicast_addr);
    printf("  → BLE MAC address: %01x:%01x:%01x:%01x:%01x:%01x\n", _device_address[5],
        _device_address[4], _device_address[3], _device_address[2], _device_address[1], _device_address[0]);
    printf("  → group address: 0x%04x\n", badge_network_info.group_addr);
    printf("  → name: %s\n", badge_network_info.name);

    return ESP_OK;
}

static int help_cmd(int argc, char **argv)
{
    /* Print summary of each command */
    for(int i=0; i<(sizeof(subcommands) / sizeof(subcommands[0])); i++) {
        if (subcommands[i].help == NULL) {
            continue;
        }
        /* First line: command name and hint
        * Pad all the hints to the same column
        */
        const char *hint = (subcommands[i].hint) ? subcommands[i].hint : "";
        printf(MESH_ADMIN_CMD " %-s %s\n", subcommands[i].command, hint);
        /* Second line: print help.
        * Argtable has a nice helper function for this which does line
        * wrapping.
        */
        printf("  "); // arg_print_formatted does not indent the first line
        arg_print_formatted(stdout, 2, 78, subcommands[i].help);
        /* Finally, print the list of arguments */
        if (subcommands[i].argtable) {
            arg_print_glossary(stdout, (void **) subcommands[i].argtable, "  %12s  %s\n");
        }
        printf("\n");
    }

    return ESP_OK;
}

static esp_err_t print_census_response(uint16_t addr, census_device_type_t type)
{
    printf("Census response from 0x%04x (%s)\n", addr, type == 0 ? "screen" : "badge");
    return ESP_OK;
}

static int census_cmd(int argc, char **argv)
{
    printf("Census will run for %u seconds\n\n", CENSUS_DEFAULT_TIMEOUT_SECONDS);

    send_census_request(&print_census_response);

    /* wait for a few seconds */
    vTaskDelay((CENSUS_DEFAULT_TIMEOUT_SECONDS * 1000) / portTICK_PERIOD_MS);

    stop_census();

    printf("Census done, %u nodes responded, %u screens, %u badges\n", census.seen,
        census.types_seen[census_device_type::screen], census.types_seen[census_device_type::badge]);

    return ESP_OK;
}

static int ping_cmd(int argc, char **argv)
{
    uint16_t address = 0;

    if (sscanf(argv[1], "0x%04hx", &address) != 1) {
        printf("address not valid (format is 0xffff)\n");
        return ESP_FAIL;
    }

    send_ping(address, console_task_handle);

    /* block until pong is received, or until time is expired */
    if(xTaskNotifyWait(0xffffffff, 0, NULL, pdMS_TO_TICKS(5 * 1000)) == pdTRUE) {
        printf("Pong received\n");
    } else {
        printf("Ping timed out (waited 5 seconds)...\n");
    }

    return ESP_OK;
}

static int info_cmd(int argc, char **argv)
{
    uint16_t address = 0;

    if (sscanf(argv[1], "0x%04hx", &address) != 1) {
        printf("address not valid (format is 0xffff)\n");
        return ESP_FAIL;
    }

    send_info_request(address, console_task_handle);

    /* block until response is received, or until time is expired */
    info_response_data_t *resp = &info_request.response;
    if(xTaskNotifyWait(0xffffffff, 0, NULL, pdMS_TO_TICKS(5 * 1000)) == pdTRUE) {
        printf("mac=%02x:%02x:%02x:%02x:%02x:%02x name='%s'\n",
            resp->mac_addr[5], resp->mac_addr[4], resp->mac_addr[3],
            resp->mac_addr[2], resp->mac_addr[1], resp->mac_addr[0],
            resp->name);
    } else {
        printf("Info request timed out (waited 5 seconds)...\n");
    }

    return ESP_OK;
}

static int set_name_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &set_name_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_name_args.end, argv[0]);
        return 1;
    }

    const char *s_address = set_name_args.address->sval[0];
    const char *name = set_name_args.name->sval[0];
    uint16_t address = 0;

    if (sscanf(s_address, "0x%04hx", &address) != 1) {
        printf("address not valid (format is 0xffff)\n");
        return ESP_FAIL;
    }

    if (strlen(name) >= BADGE_NAME_LEN) {
        printf("address not valid (maximum %u characters)\n", BADGE_NAME_LEN);
        return ESP_FAIL;
    }

    send_set_name(address, (char *)name);

    return ESP_OK;
}

static int ui_message_cmd(int argc, char **argv)
{
    uint16_t address = 0;

    if(argc != 3) {
        printf("Invalid message argument. Example: mesh-admin ui-message 0xffff \"hello this is dog\"\n");
        return ESP_OK;
    }

    if(0 == strcasecmp(argv[1], "*")) {
        address = badge_network_info.group_addr;
    } if (sscanf(argv[1], "0x%04hx", &address) != 1) {
        printf("address not valid (format is 0xffff)\n");
        return ESP_FAIL;
    }

    send_ui_message(address, argv[2]);

    return ESP_OK;
}

static int neopixel_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &neopixel_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, neopixel_args.end, argv[0]);
        return 1;
    }

    uint16_t time = neopixel_args.time->count == 0 ? 10 : neopixel_args.time->ival[0];
    if(time < 10) time = 10;
    uint8_t mode = neopixel_args.mode->ival[0];
    uint8_t brightness = neopixel_args.brightness->count == 1 ? neopixel_args.brightness->ival[0] : 128;
    uint32_t color = neopixel_args.color->count == 1 ? neopixel_args.color->ival[0] : 0;
    int ttl = neopixel_args.ttl->count == 1 ? neopixel_args.ttl->ival[0] : 1;
    bool all_modes = neopixel_args.all_modes->count == 1;

    if(color) {
        color &= 0xffffff;
    } else if (neopixel_args.red->count == 1 || neopixel_args.green->count == 1 || neopixel_args.blue->count == 1) {
        int r = neopixel_args.red->count == 1 ? neopixel_args.red->ival[0] : 0;
        int g = neopixel_args.green->count == 1 ? neopixel_args.green->ival[0] : 0;
        int b = neopixel_args.blue->count == 1 ? neopixel_args.blue->ival[0] : 0;

        color = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    } else {
        printf("at least one of --color, --red, --green or --blue must be specified\n");
        return ESP_FAIL;
    }

    printf("\nLooping forever, press any key to interrupt");
    fflush(stdout);

    do {
        int flags = NEOPIXEL_FLAG_HIGH_PRIORITY;
        if(all_modes)
            flags |= NEOPIXEL_FLAG_UNLOCK_ALL_MODES;
        send_neopixel_set(time, mode, brightness, color, flags, ttl);

        uint8_t chr;
        if(uart_read_bytes(CONFIG_ESP_CONSOLE_UART_NUM, &chr, 1, ((time - 2) * 1000) / portTICK_PERIOD_MS) == 1) {
            printf("\n");
            break;
        }

        printf(".");
        fflush(stdout);
    } while(true);

    return ESP_OK;
}

static int time_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &time_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, time_args.end, argv[0]);
        return 1;
    }

    int now = time_args.now->ival[0];
    BadgeMesh::getInstance().networkTimeSet(now);

    return ESP_OK;
}

static int flag_request_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &flag_request_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, flag_request_args.end, argv[0]);
        return 1;
    }

    uint16_t address = flag_request_args.address->sval[0];
    send_flag_request(address);

    /* wait for a few seconds for a response */
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    return ESP_OK;
}

static int mesh_admin_cmd(int argc, char **argv)
{
    if(argc < 2) {
        printf("mesh-admin requires an argument, try 'mesh-admin help'\n");
        return ESP_OK;
    }

    for(int i=0; i<(sizeof(subcommands) / sizeof(subcommands[0])); i++) {
        if(0 == strcasecmp(argv[1], subcommands[i].command)) {
            return subcommands[i].func(argc - 1, &argv[1]);
        }
    }

    printf("mesh-admin %s is not a command, try 'mesh-admin help'\n", argv[1]);
    return ESP_OK;
}

#define VALUE(string) #string
#define TO_LITERAL(string) VALUE(string)

void register_mesh_admin_commands(void)
{
    const esp_console_cmd_t cmd = {
        .command = MESH_ADMIN_CMD,
        .help = "Access admin commands for the mesh network",
        .hint = NULL,
        .func = &mesh_admin_cmd,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    set_name_args.address = arg_str1(NULL, NULL, "<addr>", "address of the node (format: 0xffff)");
    set_name_args.name = arg_str1(NULL, NULL, "<name>", "new name (maximum " TO_LITERAL(BADGE_NAME_LEN) " characters)");
    set_name_args.end = arg_end(2);

    flag_request_args.address = arg_int1(NULL, NULL, "<addr>", "address of the node (format: 0xffff)");
    flag_request_args.end = arg_end(2);

    neopixel_args.time = arg_int0("t", "time", "<int>", "number of seconds to display the pattern for (loops until interrupted unless specified)");
    neopixel_args.mode = arg_int1("m", "mode", "<int>", "number representing the mode to set (badge must support it)");
    neopixel_args.brightness = arg_int0(NULL, "brightness", "<int>", "brightness value (0-255, default 128)");
    neopixel_args.color = arg_int0("c", "color", "0xrrggbb", "hex color value (mutually exclusive with red/green/blue options)");
    neopixel_args.red = arg_int0("r", "red", "<int>", "red color value (0-255)");
    neopixel_args.green = arg_int0("g", "green", "<int>", "green color value (0-255)");
    neopixel_args.blue = arg_int0("b", "blue", "<int>", "blue color value (0-255)");
    neopixel_args.ttl = arg_int0(NULL, "ttl", "<int>", "number of mesh hops before message is no longer relayed (default 1 hop)");
    neopixel_args.all_modes = arg_lit0("a", "all", "unlock all modes, not just those unlocked by default.");
    neopixel_args.end = arg_end(15);

    time_args.now = arg_int1(NULL, NULL, "<now>", "Current number of second since epoch."); // python3 -c 'import time; print(int(time.time()))'
    time_args.end = arg_end(2);

    return;
}

#endif /* CONFIG_BADGE_MESH_ADMIN_COMMANDS */
