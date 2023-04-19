#include <string.h>
#include <esp_system.h>
#include <esp_log.h>

#include "esp_ble_mesh_defs.h"

#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/ping.h"
#include "badge/mesh/ops/pong.h"

static const char *TAG = "badge/mesh";

esp_err_t send_pong(uint16_t addr, uint64_t ping_idx)
{
    esp_err_t err;
    ping_data_t pong = {
        .src_node_addr = badge_network_info.unicast_addr,
        .idx = ping_idx,
    };

    memcpy(&pong.src_mac_addr, &_device_address, sizeof(_device_address));

    ESP_LOGV(TAG, "Sending pong in response to node=0x%02x idx=%llu", addr, ping_idx);

    err = mesh_server_send(addr, OP_VND_PONG, (uint8_t *)&pong, sizeof(pong));
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: Send pong failed (err %u)", __func__, err);
        return err;
    }

    return ESP_OK;
}

esp_err_t pong_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    ping_data_t *pong = (ping_data_t *)buf->data;
	if (ctx->addr == model->element->element_addr) {
        ESP_LOGV(TAG, "%s: Ignoring message from self", __func__);
		return ESP_OK;
	}

    ESP_LOGV(TAG, "Received pong from node=0x%02x rssi=%d mac=" MAC_STR_FMT " idx=%llu",
        pong->src_node_addr, ctx->recv_rssi, MAC_BYTES(pong->src_mac_addr), pong->idx);

    return ESP_OK;
}
