#include <string.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_random.h>

#include "esp_ble_mesh_defs.h"

#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/time.h"

static const char *TAG = "badge/mesh";

esp_err_t send_time_response(uint16_t addr, time_t now)
{
    esp_err_t err;
    time_response_data_t data = { .now = now };

    ESP_LOGV(TAG, "Sending time response to node=0x%02x", addr);

    err = BadgeMesh::getInstance().clientSend(addr, OP_VND_TIME_RESPONSE, (uint8_t *)&data, sizeof(data), false);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed (err %u)", __func__, err);
        return err;
    }

    return ESP_OK;
}

esp_err_t time_response_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    time_response_data_t *data = (time_response_data_t *)buf->data;
	if (ctx->addr == model->element->element_addr) {
        ESP_LOGV(TAG, "%s: Ignoring message from self", __func__);
		return ESP_OK;
	}

    BadgeMesh::getInstance().networkTimeSet(data->now);

    ESP_LOGV(TAG, "Received time response from node=0x%02x rssi=%d", ctx->addr, ctx->recv_rssi);

    return ESP_OK;
}

esp_err_t send_time_request()
{
    time_request_data_t data = {
        .uid = esp_random(),
    };
    esp_err_t err;

    ESP_LOGV(TAG, "Sending time request");

    err = BadgeMesh::getInstance().clientSend(NETWORK_GROUP_ADDR, OP_VND_TIME_REQUEST, (uint8_t *)&data, sizeof(data), false);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed", __func__);
        return err;
    }

    return ESP_OK;
}

esp_err_t time_request_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    time_t time;
	if (ctx->addr == model->element->element_addr) {
        ESP_LOGV(TAG, "%s: Ignoring message from self", __func__);
		return ESP_OK;
	}

    ESP_LOGV(TAG, "Received time request from node=0x%02x rssi=%d", ctx->addr, ctx->recv_rssi);

    if(ESP_OK == BadgeMesh::getInstance().networkTimeGet(&time)) {
        /* try avoiding congesting network */
        vTaskDelay((esp_random() % 1000) / portTICK_PERIOD_MS);

        send_time_response(ctx->addr, time);
    }

    return ESP_OK;
}
