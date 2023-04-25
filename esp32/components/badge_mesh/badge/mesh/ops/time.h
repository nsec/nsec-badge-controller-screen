#pragma once

#include <esp_system.h>

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OP_TIME_REQUEST                 0x20 /* must be unique */
#define OP_VND_TIME_REQUEST             ESP_BLE_MESH_MODEL_OP_3(OP_TIME_REQUEST, NSEC_COMPANY_ID)
#define OP_TIME_RESPONSE                0x21
#define OP_VND_TIME_RESPONSE            ESP_BLE_MESH_MODEL_OP_3(OP_TIME_RESPONSE, NSEC_COMPANY_ID)

typedef struct time_request_data {
    uint32_t uid; // random uid for this particular request
} time_request_data_t;

typedef struct time_response_data {
    time_t now;
} time_response_data_t;

esp_err_t send_time_request();
esp_err_t time_request_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

esp_err_t send_time_response(uint16_t addr, time_t now);
esp_err_t time_response_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

#ifdef __cplusplus
}
#endif
