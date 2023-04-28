#pragma once

#include <esp_system.h>
#include "freertos/timers.h"

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OP_NEOPIXEL_SET                   0x30 /* must be unique */
#define OP_VND_NEOPIXEL_SET               ESP_BLE_MESH_MODEL_OP_3(OP_NEOPIXEL_SET, NSEC_COMPANY_ID)

#define NEOPIXEL_FLAG_HIGH_PRIORITY (1 << 0) // admin patterns have higher priority
#define NEOPIXEL_FLAG_UNLOCK_ALL_MODES (1 << 1) // unlock all the modes from underlying FX library, not just the ~10 exposed by default

typedef struct neopixel_set_data {
    uint16_t time;
    uint8_t flags;
    uint8_t mode;
    uint8_t brightness;
    uint32_t color;
} neopixel_set_data_t;

esp_err_t send_neopixel_set(
    uint16_t time, // min. time to hold this new pattern for (in seconds)
    uint8_t mode, // which mode to enable
    uint8_t brightness, // brightness to set
    uint32_t color, // color to set
    uint8_t flags, // zero or more of NEOPIXEL_FLAG_* above
    uint8_t ttl // ttl for message over mesh network
);
esp_err_t neopixel_set_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

#ifdef __cplusplus
}
#endif
