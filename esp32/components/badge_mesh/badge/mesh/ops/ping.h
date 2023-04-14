#pragma once

#include <esp_system.h>

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#define OP_PING                 0xbb /* must be unique */
#define OP_VND_PING             ESP_BLE_MESH_MODEL_OP_3(OP_PING, NSEC_COMPANY_ID)

#define MAC_STR_FMT "%01x:%01x:%01x:%01x:%01x:%01x"
#define MAC_BYTES(m) m[5], m[4], m[3], m[2], m[1], m[0]

typedef struct ping_data {
    uint8_t src_mac_addr[6];
    uint16_t src_node_addr;

    // incremental ping index
    uint64_t idx;
} ping_data_t;

esp_err_t send_ping(uint64_t idx);
esp_err_t ping_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);
