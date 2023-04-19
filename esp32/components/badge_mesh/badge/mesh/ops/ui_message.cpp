#include <string.h>
#include <esp_system.h>
#include <esp_log.h>

#include "esp_ble_mesh_defs.h"

#include "display.h"
#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/ui_message.h"

static const char *TAG = "badge/mesh";

#if CONFIG_BADGE_MESH_ADMIN_COMMANDS

esp_err_t send_ui_message(uint16_t addr, char *msg)
{
    esp_err_t err;

    ESP_LOGV(TAG, "%s: node=0x%04x msg='%s'", __func__, addr, msg);

    err = mesh_client_send(addr, OP_VND_UI_MESSAGE, (uint8_t *)msg, strlen(msg)+1, false);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed", __func__);
        return err;
    }

    return ESP_OK;
}

#endif /* CONFIG_BADGE_MESH_ADMIN_COMMANDS */

esp_err_t ui_message_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    char *msg = (char *)buf->data;
	if (ctx->addr == model->element->element_addr) {
        ESP_LOGV(TAG, "%s: Ignoring message from self", __func__);
        return ESP_OK;
    }

    ESP_LOGV(TAG, "Received ui message from node=0x%04x msg='%s'", ctx->addr, msg);

    Display::getInstance().showMessage(msg);

    return ESP_OK;
}
