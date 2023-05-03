#include <string.h>
#include <esp_random.h>
#include "cmd_flag.h"

#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_log.h"

#define FLAG_1_NAME "/flags/data/flag.txt"
#define FLAG_2_NAME "/flags/flag.txt"

static int flag_cmd(int argc, char **argv)
{
    FILE *f;

    printf("Writing " FLAG_1_NAME " ...");
    f = fopen(FLAG_1_NAME, "w");
    if (f == NULL) {
        printf("Failed to open file for writing (directory may not exist?)\n");
        return ESP_FAIL;
    }
    fprintf(f, "FLAG-{768a217e380b516f651939f67f8922a8}\n");
    fclose(f);
    printf("ok\n");

    printf("Writing " FLAG_2_NAME " ...");
    f = fopen(FLAG_2_NAME, "w");
    if (f == NULL) {
        printf("Failed to open file for writing (directory may not exist?)\n");
        return ESP_FAIL;
    }
    fprintf(f, "FLAG-{5418f0cfd7d67de7cbafbdffd29f3f3d}\n");
    fclose(f);
    printf("ok\n");

    return 0;
}

void register_flag_commands(void)
{
    const esp_console_cmd_t cmd = {
        .command = "flag",
        .help = "Do the thing",
        .hint = NULL,
        .func = &flag_cmd,
        .argtable = NULL,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
