#include <string.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_random.h>

#include "esp_ble_mesh_defs.h"

#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/realtimeled.h"

static const char *TAG = "badge/mesh/rtled";

#if CONFIG_BADGE_MESH_ADMIN_COMMANDS

esp_err_t send_rtled_request(uint16_t start, uint32_t ledids, uint8_t color, uint8_t brightness)
{
    esp_err_t err;

    rtled_request_data_t data = {
        .start = start,
        .ledids = ledids,
        .color = color,
        .brightness = brightness,
    };

    err = mesh_client_send(NETWORK_GROUP_ADDR, OP_VND_RTLED_REQUEST, (uint8_t *)&data, sizeof(data), false);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed", __func__);
        return err;
    }

    ESP_LOGW(TAG, "Sending rtled start=%d", data.start);

    return ESP_OK;
}

esp_err_t rtled_request_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    rtled_request_data_t *data = (rtled_request_data_t *)buf->data;

    ESP_LOGV(TAG, "%s from node=0x%04x start=%d ledids:0x%lu color:0x%u brightness:%d", __func__, ctx->addr, data->start, data->ledids, data->color, data->brightness);

	if (ctx->addr == model->element->element_addr) {
        ESP_LOGV(TAG, "%s: Ignoring message from self", __func__);
		return ESP_OK;
	}

    return ESP_OK;
}

#endif /* CONFIG_BADGE_MESH_ADMIN_COMMANDS */

