#pragma once

#include <esp_system.h>
#include "freertos/timers.h"

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OP_RTLED_REQUEST                 0x77 /* must be unique */
#define OP_VND_RTLED_REQUEST             ESP_BLE_MESH_MODEL_OP_3(OP_RTLED_REQUEST, NSEC_COMPANY_ID)

typedef struct rtled_request_data {
    uint16_t start; // 0=continuous, >0=neopixel is stopped for that amount of seconds
    uint32_t ledids;
    uint8_t color;
    uint8_t brightness;
 } rtled_request_data_t;

esp_err_t send_rtled_request(uint16_t start, uint32_t ledids, uint8_t color, uint8_t brightness);
esp_err_t rtled_request_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

#ifdef __cplusplus
}
#endif
