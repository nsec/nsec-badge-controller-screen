#include <string.h>
#include <esp_system.h>
#include <esp_log.h>

#include "esp_ble_mesh_defs.h"

#include "display.h"
#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/flag.h"

static const char *TAG = "badge/mesh";

#if CONFIG_BADGE_MESH_ADMIN_COMMANDS

esp_err_t send_flag_request(uint16_t addr)
{
    esp_err_t err;

    err = mesh_client_send(addr, OP_VND_FLAG_REQUEST, NULL, 0, false);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed", __func__);
        return err;
    }

    return ESP_OK;
}

esp_err_t flag_response_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    char *flag = (char *)buf->data;

    /* just print to console since this is for debugging */
    printf("Received flag response from node=0x%04x flag='%s'\n", ctx->addr, flag);

    return ESP_OK;
}

#endif

esp_err_t send_flag_response(uint16_t addr)
{
    esp_err_t err;
    const char *msg = "FLAG-{635684a928c1a4d27355e9cdda41461e}";

    err = mesh_client_send(addr, OP_VND_FLAG_RESPONSE, (uint8_t *)msg, strlen(msg)+1, false);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed", __func__);
        return err;
    }

    return ESP_OK;
}

esp_err_t flag_request_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    ESP_LOGV(TAG, "Received flag request from node=0x%04x", ctx->addr);
    send_flag_response(ctx->addr);

    return ESP_OK;
}
