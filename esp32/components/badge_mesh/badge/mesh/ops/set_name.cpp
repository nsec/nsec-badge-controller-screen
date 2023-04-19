#include <string.h>
#include <esp_system.h>
#include <esp_log.h>

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_networking_api.h"

#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/set_name.h"

static const char *TAG = "badge/mesh";

#if CONFIG_BADGE_MESH_ADMIN_COMMANDS

esp_err_t send_set_name(uint16_t addr, char *name)
{
    esp_err_t err;
	esp_ble_mesh_msg_ctx_t ctx = {
		.net_idx = badge_network_info.net_idx,
		.app_idx = badge_network_info.app_idx,
		.addr = addr,
		.send_ttl = DEFAULT_TTL,
	};

    set_name_data_t data = {};

    memset(&data, 0, sizeof(data));
    snprintf((char *)&data.name, sizeof(data.name), "%s", name);

    ESP_LOGV(TAG, "%s: node=0x%04x name='%s'", __func__, addr, name);

    err = mesh_client_send(addr, OP_VND_SET_NAME, (uint8_t *)&data, sizeof(data), false);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed", __func__);
        return err;
    }

    return ESP_OK;
}

#endif /* CONFIG_BADGE_MESH_ADMIN_COMMANDS */

esp_err_t set_name_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    set_name_data_t *data = (set_name_data_t *)buf->data;
	// if (ctx->addr == model->element->element_addr) {
    //     ESP_LOGV(TAG, "%s: Ignoring message from self", __func__);
	// 	return ESP_OK;
	// }

    ESP_LOGV(TAG, "Received new name from node=0x%04x name='%s'", ctx->addr, data->name);

    mesh_config_name_updated(data->name);

    return ESP_OK;
}
