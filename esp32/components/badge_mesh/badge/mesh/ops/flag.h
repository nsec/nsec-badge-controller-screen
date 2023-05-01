#pragma once

#include <esp_system.h>

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OP_FLAG_REQUEST                     0x10 /* must be unique */
#define OP_VND_FLAG_REQUEST                 ESP_BLE_MESH_MODEL_OP_3(OP_FLAG_REQUEST, NSEC_COMPANY_ID)
#define OP_FLAG_RESPONSE                    0x11 /* must be unique */
#define OP_VND_FLAG_RESPONSE                ESP_BLE_MESH_MODEL_OP_3(OP_FLAG_RESPONSE, NSEC_COMPANY_ID)

esp_err_t send_flag_request(uint16_t addr);
esp_err_t send_flag_response(uint16_t addr);
esp_err_t flag_request_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);
esp_err_t flag_response_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

#ifdef __cplusplus
}
#endif
