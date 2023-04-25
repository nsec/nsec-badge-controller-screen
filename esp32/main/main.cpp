#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "nvs_flash.h"
#include "console.h"
#include "neopixel.h"
#include "buzzer.h"
#include "display.h"
#include "disk.h"
#include "save.h"
#include "badge/mesh/main.h"

static void initialize_nvs(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

extern "C" void app_main(void) {

    initialize_nvs();
    fflush(stdout);

    setenv("TZ", "UTC+4", 1);

    Save::load_save();
    Save::load_and_set_log_levels();

    /* will only show once log_level is saved and board is restarted */
    ESP_LOGI("flag", "🤔 " LOG_COLOR("38;5;232;48;5;232") "FLAG-{378792f89d19dfe064b5fa36b5c54971}" LOG_RESET_COLOR);

	NeoPixel::getInstance().init();

    Buzzer::getInstance().init();
    Buzzer::getInstance().play(Buzzer::Sounds::Mode1);

    Display::getInstance().init();

    Disk::getInstance().init();

    BadgeMesh::getInstance().init();

    console_create_task();
}
