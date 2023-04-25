#include <string.h>
#include <esp_random.h>
#include "cmd_debug.h"

#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_log.h"
#include "save.h"

static struct {
    struct arg_str *cmd;
    struct arg_int *pin;
    struct arg_end *end;
} args;

static int debug_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&args);
    if (nerrors != 0) {
        arg_print_errors(stderr, args.end, argv[0]);
        return 1;
    }
    int pin = args.pin->ival[0];
    bool enabled;

    if(!strcasecmp(args.cmd->sval[0], "enable")) {
        enabled = true;
    } else if(!strcasecmp(args.cmd->sval[0], "disable")) {
        enabled = false;
    } else {
        printf("Unknown debug command, either 'enable' or 'disable' is supported.\n");
        return ESP_FAIL;
    }

    if(pin == Save::save_data.debug_pin) {
        Save::save_data.debug_enabled = enabled;
        Save::write_save();
        printf("%s debug mode, restart for changes to take effect.\n", enabled ? "Enabled" : "Disabled");
        if(enabled) {
            printf("Congratulations, here is your FLAG-{590b65680b892dae31181bdfcfcf33a3}\n");
        }
    } else {
        printf("Debug mode not %s: incorrect pin.\n", enabled ? "enabled" : "disabled");
        return ESP_FAIL;
    }

    return 0;
}

void register_debug_commands(void)
{
    args.cmd = arg_str1(NULL, NULL, "<cmd>", "what to do, either 'enable' or 'disable'");
    args.pin = arg_int1(NULL, NULL, "<pin>", "secure pin for unlocking debug mode");
    args.end = arg_end(3);

    if (Save::save_data.debug_pin == 0) {
        Save::save_data.debug_pin = esp_random() % 9999;
        Save::write_save();
    }

    const esp_console_cmd_t cmd = {
        .command = "debug",
        .help = "Enable debug mode",
        .hint = NULL,
        .func = &debug_cmd,
        .argtable = &args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
