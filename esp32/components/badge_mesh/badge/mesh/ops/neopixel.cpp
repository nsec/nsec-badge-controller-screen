#include <string.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_random.h>

#include "esp_ble_mesh_defs.h"

#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/neopixel.h"

static const char *TAG = "badge/mesh";

esp_err_t send_neopixel_set(uint16_t time, uint8_t mode, uint8_t brightness, uint32_t color, uint8_t flags, uint8_t ttl)
{
    esp_err_t err;

    neopixel_set_data_t data = {
        .time = time,
        .flags = flags,
        .mode = mode,
        .brightness = brightness,
        .color = color,
    };

    err = BadgeMesh::getInstance().clientSend(NETWORK_GROUP_ADDR, OP_VND_NEOPIXEL_SET,
        (uint8_t *)&data, sizeof(data), false, ttl);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed", __func__);
        return err;
    }

    ESP_LOGV(TAG, "Sent neopixel set command");

    return ESP_OK;
}

esp_err_t neopixel_set_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    /* do nothing for controller screen */
    return ESP_OK;
}
