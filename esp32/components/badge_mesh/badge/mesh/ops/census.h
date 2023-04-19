#pragma once

#include <esp_system.h>
#include "freertos/timers.h"

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OP_CENSUS_REQUEST                 0x03 /* must be unique */
#define OP_VND_CENSUS_REQUEST             ESP_BLE_MESH_MODEL_OP_3(OP_CENSUS_REQUEST, NSEC_COMPANY_ID)
#define OP_CENSUS_RESPONSE                0x04 /* must be unique */
#define OP_VND_CENSUS_RESPONSE            ESP_BLE_MESH_MODEL_OP_3(OP_CENSUS_RESPONSE, NSEC_COMPANY_ID)

#define CENSUS_DEFAULT_TIMEOUT_SECONDS    30

typedef struct census_request_data {
    uint32_t uid; /* unique census ID, in case more than one is running at the same time */
} census_request_data_t;

typedef struct census_response_data {
    uint32_t uid; /* unique census ID, in case more than one is running at the same time */
} census_response_data_t;

typedef esp_err_t (* census_response_cb_t)(uint16_t addr);
typedef esp_err_t (* census_done_cb_t)(unsigned int seen);

typedef struct census_ctx {
    uint32_t uid; /* uid of the current census */
    bool in_progress; /* true if currently in progress */
    unsigned int timeout; /* time to wait for all responses to be received */
    TimerHandle_t done_timer;
    census_response_cb_t resp_cb; /* called when a response is received */
    census_done_cb_t done_cb; /* called when the timeout is passed */

    unsigned int seen;
} census_ctx_t;

extern census_ctx_t census;

/*
    Send the "census request" command to all nodes in the network group.

    'timeout': how long to wait for all responses to be received (default CENSUS_DEFAULT_TIMEOUT_SECONDS)
*/
esp_err_t send_census_request(unsigned int timeout, census_response_cb_t resp_cb, census_done_cb_t done_cb);
esp_err_t census_request_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

/*
    Stop census, if one is in progress.
*/
void stop_census();

/*
    Send the "census response" command to all nodes in the network group.

    'timeout': how long to wait for all responses to be received (default CENSUS_DEFAULT_TIMEOUT_SECONDS)
*/
esp_err_t send_census_response(uint16_t dst, uint32_t uid);
esp_err_t census_response_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

#ifdef __cplusplus
}
#endif
