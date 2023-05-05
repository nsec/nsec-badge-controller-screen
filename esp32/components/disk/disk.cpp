
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include <esp_log.h>
#include "diskio_impl.h"
#include "diskio_sdmmc.h"

#include "disk.h"
#include "buzzer.h"

/*

Example reference:

https://github.com/espressif/esp-idf/blob/master/examples/storage/sd_card/sdspi/main/sd_card_example_main.c

*/

static const char *TAG = "disk";
static const char *MOUNT_POINT = "/sdcard";

#define PIN_NUM_MISO  GPIO_NUM_19
#define PIN_NUM_MOSI  GPIO_NUM_23
#define PIN_NUM_CLK   GPIO_NUM_18
#define PIN_NUM_CS    GPIO_NUM_5

#define CHECK_EXECUTE_RESULT(err, str) do { \
    if ((err) !=ESP_OK) { \
        ESP_LOGE(TAG, str" (0x%x).", err); \
        goto cleanup; \
    } \
    } while(0)

static int card_handle = -1;
static esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 1,
    .allocation_unit_size = 16 * 1024,
    .disk_status_check_enable = 1,
};
static BYTE pdrv = FF_DRV_NOT_USED;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static esp_err_t _initialize_spi_bus()
{
    esp_err_t ret;
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SOC_SPI_MAXIMUM_BUFFER_SIZE,
    };
    return spi_bus_initialize(VSPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
}

static esp_err_t _attach_spi_bus(sdspi_dev_handle_t *p_card_handle)
{
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = VSPI_HOST;

    return sdspi_host_init_device((const sdspi_device_config_t *)&slot_config, p_card_handle);
}

void Disk::init()
{
    esp_err_t ret;

    _enabled = false;
    strcpy((char *)&_mount_point, MOUNT_POINT);

    host.slot = VSPI_HOST;

    ret = _initialize_spi_bus();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = (*host.init)();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI host init failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = _attach_spi_bus(&card_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach sdspi device onto an SPI bus. (%s)", esp_err_to_name(ret));
        return;
    }

    host.slot = card_handle;
    // xTaskCreate((TaskFunction_t)&(Disk::task), "sd card", 4096, this, 5, &Disk::_taskHandle);
}

/*
static esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

static esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}
*/

static esp_err_t _mount(const esp_vfs_fat_mount_config_t *mount_config, sdmmc_card_t *card, uint8_t pdrv, char *mp)
{
    FRESULT res;
    FATFS* fs = NULL;
    esp_err_t err;
    ff_diskio_register_sdmmc(pdrv, card);
    ff_sdmmc_set_disk_status_check(pdrv, mount_config->disk_status_check_enable);
    ESP_LOGD(TAG, "using pdrv=%i", pdrv);
    char drv[3] = {(char)('0' + pdrv), ':', 0};
    // char drive_label[12];
    // uint32_t drive_sn;

    // connect FATFS to VFS
    err = esp_vfs_fat_register(mp, drv, mount_config->max_files, &fs);
    if (err == ESP_ERR_INVALID_STATE) {
        // it's okay, already registered with VFS
    } else if (err != ESP_OK) {
        ESP_LOGD(TAG, "esp_vfs_fat_register failed 0x(%x)", err);
        goto fail;
    }

    // Try to mount partition
    res = f_mount(fs, drv, 1);
    if (res != FR_OK) {
        err = ESP_FAIL;
        ESP_LOGE(TAG, "failed to mount card (%d)", res);
        goto fail;
    }

    return ESP_OK;

fail:
    if (fs) {
        f_mount(NULL, drv, 0);
    }
    esp_vfs_fat_unregister_path(mp);
    ff_diskio_unregister(pdrv);
    return err;
}

static esp_err_t _unmount(sdmmc_card_t *card, uint8_t pdrv, char *mp)
{
    if (pdrv == 0xff) {
        return ESP_ERR_INVALID_ARG;
    }

    // unmount
    char drv[3] = {(char)('0' + pdrv), ':', 0};
    FRESULT res = f_mount(0, drv, 0);
    ESP_LOGI(TAG, "%s: f_mount res=%d", __func__, res);
    // release SD driver
    ff_diskio_unregister(pdrv);

    esp_err_t err = esp_vfs_fat_unregister_path(mp);
    return err;
}

bool Disk::iterPath(const char *path, disk_iter_cb_t cb, void *param)
{
    DIR *dir;
    dirent *entry;

    dir = opendir(path);
    if(dir == NULL) {
        return false;
    }

    while ((entry = readdir(dir)) != NULL) {
        if(!cb(entry, param))
            break;
    }

    closedir(dir);
    return true;
}

static bool dir_exist(const char *path)
{
    DIR *dir = opendir(path);
    if(dir == NULL) {
        return false;
    }

    closedir(dir);
    return true;
}

void Disk::taskHandler()
{
    esp_err_t ret;

    if(!_enabled) {
        if(_cardState == CardState::NotPresent) {
            return;
        } else if(_cardState == CardState::Present || _cardState == CardState::NotReadable) {
            _cardState = CardState::Failed;
        }
    }

    char volume_label[12];
    char drv[3] = {(char)('0' + pdrv), ':', 0};

    switch(_cardState) {
    case CardState::NotPresent:
        ret = sdmmc_card_init(&host, &_card);
        if (ret != ESP_OK) {
            //ESP_LOGE(TAG, "Card isn't present (%s)", esp_err_to_name(ret));
            break;
        }

        ESP_LOGI(TAG, "SD card inserted");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Card has been initialized, print its properties
        // sdmmc_card_print_info(stdout, &_card);

        snprintf((char *)&_mount_point, sizeof(_mount_point), "%s", MOUNT_POINT);

        // get a drive where we can mount the sd card
        if (ff_diskio_get_drive(&pdrv) != ESP_OK || pdrv == FF_DRV_NOT_USED) {
            ESP_LOGE(TAG, "the maximum count of volumes is already mounted");
            _cardState = CardState::NotReadable;
            Buzzer::getInstance().buzz(900, 1000);
            break;
        }

        ret = _mount(&mount_config, &_card, pdrv, _mount_point);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to mount drive. It may be formatted incorrectly, or SD card is corrupted? (%s)", esp_err_to_name(ret));
            _cardState = CardState::NotReadable;
            Buzzer::getInstance().buzz(900, 1000);
            break;
        }

        memset(volume_label, 0, sizeof(volume_label));

        /* Get volume label of the default drive */
        drv[0] = (char)('0' + pdrv);
        if(FR_OK == f_getlabel(drv, (char *)&volume_label, NULL)) {
            ret = _unmount(&_card, pdrv, _mount_point);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to remount drive with volume label. (%s)", esp_err_to_name(ret));
                _cardState = CardState::NotReadable;
                Buzzer::getInstance().buzz(900, 1000);
                break;
            }

            snprintf((char *)&_mount_point, sizeof(_mount_point), "/%s", volume_label);

            if(dir_exist(_mount_point)) {
                ESP_LOGE(TAG, "Failed to remount drive at %s, folder already exists", _mount_point);
                _cardState = CardState::NotReadable;
                Buzzer::getInstance().buzz(900, 1000);
                break;
            }

            ret = _mount(&mount_config, &_card, pdrv, _mount_point);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to mount drive. It may be formatted incorrectly, or SD card is corrupted? (%s)", esp_err_to_name(ret));
                _cardState = CardState::NotReadable;
                Buzzer::getInstance().buzz(900, 1000);
                break;
            }
        }

        Buzzer::getInstance().play(Buzzer::Sounds::Connection);
        _cardState = CardState::Present;
        break;
    case CardState::NotReadable:
    case CardState::Present:
        // nothing to do but wait until the user removed their card
        ret = sdmmc_get_status(&_card);
        if (ret != ESP_OK) {
            ESP_LOGI(TAG, "SD card removed");

            Buzzer::getInstance().play(Buzzer::Sounds::Disconnection);
            _cardState = CardState::Failed;
        }

        break;
    case CardState::Failed:
        // do stuff to unmount the cart and deinit some things

        ESP_LOGI(TAG, "Unmounting SD card");
        ret = _unmount(&_card, pdrv, _mount_point);
        if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to unmount drive. (%s)", esp_err_to_name(ret));
        }
        _cardState = CardState::NotPresent;
        break;
    }
}
