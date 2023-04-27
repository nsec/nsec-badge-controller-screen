#pragma once

#include <esp_system.h>

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OP_UI_MESSAGE                  0x05 /* must be unique */
#define OP_VND_UI_MESSAGE              ESP_BLE_MESH_MODEL_OP_3(OP_UI_MESSAGE, NSEC_COMPANY_ID)

/*
    Send the "ui message" command to a node.

    'addr': the node address to update
    'name': the new name
*/
esp_err_t send_ui_message(uint16_t addr, char *msg);
esp_err_t ui_message_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

#ifdef __cplusplus
}
#endif
