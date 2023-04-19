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

#include "badge/mesh/cmd/admin.h"
#include "badge/mesh/network.h"
#include "badge/mesh/host.h"
#include "badge/mesh/config.h"
#include "badge/mesh/ops/set_name.h"
#include "badge/mesh/ops/census.h"

#if CONFIG_BADGE_MESH_ADMIN_COMMANDS

static const char *TAG = "cmd_mesh_admin";
#define MESH_ADMIN_CMD "mesh-admin"

static int info_cmd(int argc, char **argv);
static int help_cmd(int argc, char **argv);
static int census_cmd(int argc, char **argv);
static int set_name_cmd(int argc, char **argv);

static struct {
    struct arg_str *address;
    struct arg_str *name;
    struct arg_end *end;
} set_name_args;

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
        .command = "info",
        .help = "Print information about the mesh",
        .hint = NULL,
        .func = &info_cmd,
    },
    {
        .command = "census",
        .help = "Start or stop a network-wide census of all nodes",
        .hint = NULL,
        .func = &census_cmd,
    },
    {
        .command = "set-name",
        .help = "Set a new name for a node in the mesh",
        .hint = NULL,
        .func = &set_name_cmd,
        .argtable = &set_name_args
    },
};

static int info_cmd(int argc, char **argv)
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

static esp_err_t print_census_response(uint16_t addr)
{
    printf("Census response from 0x%04x\n", addr);
    return ESP_OK;
}

static esp_err_t census_done(unsigned int seen)
{
    printf("Census recorded %u nodes.\n", seen);
    return ESP_OK;
}

static int census_cmd(int argc, char **argv)
{
    if(argc > 1 && !strcasecmp(argv[1], "stop")) {
        stop_census();
        return ESP_OK;
    }

    send_census_request(CENSUS_DEFAULT_TIMEOUT_SECONDS, &print_census_response, &census_done);

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

    return;
}

#endif /* CONFIG_BADGE_MESH_ADMIN_COMMANDS */
