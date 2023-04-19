#include <string.h>
#include <esp_system.h>
#include <esp_log.h>

#include "esp_ble_mesh_defs.h"

#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/ping.h"
#include "badge/mesh/ops/pong.h"

static const char *TAG = "badge/mesh";

esp_err_t send_ping(uint64_t idx)
{
    esp_err_t err;
    ping_data_t ping = {
        .src_node_addr = badge_network_info.unicast_addr,
        .idx = idx,
    };

    memcpy(&ping.src_mac_addr, &_device_address, sizeof(_device_address));

    ESP_LOGV(TAG, "Sending ping from node=0x%02x idx=%llu", ping.src_node_addr, ping.idx);

    err = mesh_client_send(NETWORK_GROUP_ADDR, OP_VND_PING, (uint8_t *)&ping, sizeof(ping), true);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: Send ping failed", __func__);
        return err;
    }

    return ESP_OK;
}

esp_err_t ping_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    ping_data_t *ping = (ping_data_t *)buf->data;
	if (ctx->addr == model->element->element_addr) {
        ESP_LOGV(TAG, "%s: Ignoring message from self", __func__);
		return ESP_OK;
	}

    ESP_LOGV(TAG, "Received ping from node=0x%02x rssi=%d mac=" MAC_STR_FMT " idx=%llu",
        ping->src_node_addr, ctx->recv_rssi, MAC_BYTES(ping->src_mac_addr), ping->idx);

    send_pong(ping->src_node_addr, ping->idx);

    return ESP_OK;
}
