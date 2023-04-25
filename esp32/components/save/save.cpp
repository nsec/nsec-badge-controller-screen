#include "save.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdio.h>

static const char *TAG="save";

#define STORAGE_NAMESPACE "storage"
#define LOG_NAMESPACE "logs"

SaveData Save::save_data = {
    .neopixel_brightness = 255,
    .neopixel_mode = 5,
    .neopixel_is_on = true,
    .neopixel_color = 0xffffff,
};

esp_err_t Save::write_save()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: nvs open failed", __func__);
        return err;
    }

    err = nvs_set_blob(my_handle, "save", reinterpret_cast<void *>(&save_data),
                       sizeof(save_data));

    if (err != ESP_OK) {
        return err;
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        return err;
    }

    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t Save::load_save()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: nvs open failed", __func__);
        return err;
    }

    size_t save_size = sizeof(save_data);
    err = nvs_get_blob(my_handle, "save", reinterpret_cast<void *>(&save_data),
                       &save_size);
    if (err != ESP_OK) {
        return err;
    }

    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t Save::clear_log_levels()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(LOG_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: nvs open failed", __func__);
        return err;
    }

    err = nvs_erase_all(my_handle);
        ESP_LOGE(TAG, "%s: nvs erase all failed", __func__);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: nvs commit failed", __func__);
        return err;
    }

    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t Save::save_log_level(const char *log, esp_log_level_t level)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(LOG_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: nvs open failed", __func__);
        return err;
    }

    err = nvs_set_u8(my_handle, log, (uint8_t)level);
    if (err != ESP_OK) {
        return err;
    }

    ESP_LOGV(TAG, "Saved log level '%u' for '%s'", (int)level, log);

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: nvs commit failed", __func__);
        return err;
    }

    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t Save::load_and_set_log_levels()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(LOG_NAMESPACE, NVS_READONLY, &my_handle);
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        // Partition does not exist, and it is not created since we asked for READONLY.
        // This is expected before name is actually set, we can silently return an error.
        return ESP_FAIL;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: nvs open failed", __func__);
        return err;
    }

    nvs_iterator_t it = NULL;
    esp_err_t res = nvs_entry_find(NVS_DEFAULT_PART_NAME, LOG_NAMESPACE, NVS_TYPE_U8, &it);
    while(res == ESP_OK) {
        nvs_entry_info_t info;
        esp_log_level_t level;

        nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
        if(nvs_get_u8(my_handle, info.key, (uint8_t *)&level) == ESP_OK) {
            esp_log_level_set(info.key, level);
            ESP_LOGV(TAG, "Loaded saved log level '%u' for '%s'", (int)level, info.key);
        }
        res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);

    nvs_close(my_handle);
    return ESP_OK;
}
