#pragma once

#include <esp_system.h>

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OP_SET_NAME                 0x02 /* must be unique */
#define OP_VND_SET_NAME             ESP_BLE_MESH_MODEL_OP_3(OP_SET_NAME, NSEC_COMPANY_ID)

typedef struct set_name_data {
    char name[BADGE_NAME_LEN];
} set_name_data_t;

/*
    Send the "set name" command to a node.

    'addr': the node address to update
    'name': the new name
*/
esp_err_t send_set_name(uint16_t addr, char *name);
esp_err_t set_name_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

#ifdef __cplusplus
}
#endif
