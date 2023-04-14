#pragma once

#include <esp_system.h>

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#define OP_PONG                 0xbd
#define OP_VND_PONG             ESP_BLE_MESH_MODEL_OP_3(OP_PONG, NSEC_COMPANY_ID)

esp_err_t send_pong(uint16_t addr, uint64_t ping_idx);
esp_err_t pong_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);
