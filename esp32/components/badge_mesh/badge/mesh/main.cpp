#include <esp_system.h>
#include <esp_log.h>
#include "esp_ble_mesh_networking_api.h"
#include <sys/time.h>

#include "save.h"
#include "badge/mesh/main.h"
#include "badge/mesh/host.h"
#include "badge/mesh/ops/time.h"
#include "badge/mesh/ops/partyline.h"

static const char *TAG = "badge/mesh";

// TaskHandle_t BadgeMesh::_taskHandle = NULL;

void BadgeMesh::init()
{
    networkTimeValid = false;
    _bt_semaphore = xSemaphoreCreateMutex();
    _enabled = false;
    _state = State::Disabled;

    partyline_history_init();

    if(Save::save_data.debug_feature_enabled[debug_tab::mesh] && Save::save_data.debug_feature_enabled[debug_tab::wifi]) {
        Save::save_data.debug_feature_enabled[debug_tab::mesh] = false;
    }

    if(Save::save_data.debug_feature_enabled[debug_tab::mesh]) {
        enable();
    } else {
        disable();
    }
}

esp_err_t BadgeMesh::clientSend(uint16_t dst_addr, uint32_t op, uint8_t *msg, unsigned int length, bool needsResponse, uint8_t ttl)
{
    if(_state != State::Enabled) {
        ESP_LOGV(TAG, "Mesh not enabled (or failed)");
        return ESP_FAIL;
    }

    esp_err_t err;
	esp_ble_mesh_msg_ctx_t ctx = {
		.net_idx = badge_network_info.net_idx,
		.app_idx = badge_network_info.app_idx,
		.addr = dst_addr,
        .send_rel = 1,
		.send_ttl = ttl,
	};

	if (xSemaphoreTake(_bt_semaphore, (TickType_t)9999) != pdTRUE) {
        ESP_LOGE(TAG, "%s: Could not aquire semaphore", __func__);
        return ESP_FAIL;
    }

    err = esp_ble_mesh_client_model_send_msg(cli_vnd_model, &ctx, op, length, msg, 0, needsResponse, ROLE_NODE);
    mesh_sequence_number_changed();
    xSemaphoreGive(_bt_semaphore);

    if(err != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not client_modelsend", __func__);
        return ESP_FAIL;
    }

    return err;
}

esp_err_t BadgeMesh::serverSend(uint16_t dst_addr, uint32_t op, uint8_t *msg, unsigned int length)
{
    if(_state != State::Enabled) {
        ESP_LOGV(TAG, "Mesh not enabled (or failed)");
        return ESP_FAIL;
    }

    esp_err_t err;
	esp_ble_mesh_msg_ctx_t ctx = {
		.net_idx = badge_network_info.net_idx,
		.app_idx = badge_network_info.app_idx,
		.addr = dst_addr,
        .send_rel = 1,
		.send_ttl = DEFAULT_TTL,
	};

	if (xSemaphoreTake(_bt_semaphore, (TickType_t)9999) != pdTRUE) {
        ESP_LOGE(TAG, "%s: Could not aquire semaphore", __func__);
        return ESP_FAIL;
    }

    err = esp_ble_mesh_server_model_send_msg(srv_vnd_model, &ctx, op, length, msg);
    mesh_sequence_number_changed();
    xSemaphoreGive(_bt_semaphore);

    if(err != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not server_model_send", __func__);
        return ESP_FAIL;
    }
    return err;
}

esp_err_t mesh_client_send(uint16_t dst_addr, uint32_t op, uint8_t *msg, unsigned int length, bool needsResponse)
{
    return BadgeMesh::getInstance().clientSend(dst_addr, op, msg, length, needsResponse);
}

esp_err_t mesh_server_send(uint16_t dst_addr, uint32_t op, uint8_t *msg, unsigned int length)
{
    return BadgeMesh::getInstance().serverSend(dst_addr, op, msg, length);
}

bool BadgeMesh::networkTimeIsValid()
{
    return networkTimeValid;
}

esp_err_t BadgeMesh::networkTimeSet(time_t now)
{
    struct timeval t = {
        .tv_sec = now,
        .tv_usec = 0,
    };

    if(settimeofday(&t, NULL) != 0) {
        return ESP_FAIL;
    }

    networkTimeValid = true;
    return ESP_OK;
}

esp_err_t BadgeMesh::networkTimeGet(time_t *now)
{
    struct timeval t;

    if(!networkTimeValid) {
        return ESP_FAIL;
    }

    if(gettimeofday(&t, NULL) != 0) {
        return ESP_FAIL;
    }

    *now = t.tv_sec;
    return ESP_OK;
}

esp_err_t BadgeMesh::networkTimeRequest()
{
    if(_state != State::Enabled) {
        ESP_LOGV(TAG, "Mesh not enabled (or failed)");
        return ESP_OK;
    }

    send_time_request();
    return ESP_OK;
}

esp_err_t BadgeMesh::enable()
{
    esp_err_t err;

    if(_enabled) {
        ESP_LOGV(TAG, "Already enabled...");
        return ESP_OK;
    }

    ESP_LOGV(TAG, "Enabling...");

    _enabled = true;

	if (xSemaphoreTake(_bt_semaphore, (TickType_t)9999) == pdTRUE) {
        err = mesh_host_initialize();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize NimBLE (%s)", esp_err_to_name(err));
            goto fail;
        }

        while(!mesh_host_initialized)
        {
            // Wait for the async mesh initialization process to finish
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        ESP_LOGV(TAG, "Mesh host has been initialized");

        err = mesh_configure_esp_ble_mesh();
        if (err) {
            ESP_LOGE(TAG, "Configuring mesh failed (err %s)", esp_err_to_name(err));
            goto fail;
        }

        if (!mesh_is_provisioned()) {
            ESP_LOGE(TAG, "This node is not properly initialized to be part of a mesh");
            goto fail;
        } else {
            ESP_LOGV(TAG, "This node is provisioned correctly");
        }

        xSemaphoreGive(_bt_semaphore);
    }

    _state = State::Enabled;

    return ESP_OK;
fail:
    mesh_deconfigure_esp_ble_mesh();
    mesh_host_deinit();

    xSemaphoreGive(_bt_semaphore);

    _state = State::Failed;
    return ESP_FAIL;
}

esp_err_t BadgeMesh::disable()
{
    esp_err_t err;

    if(!_enabled) {
        ESP_LOGV(TAG, "Already disabled...");
        return ESP_OK;
    }

    ESP_LOGV(TAG, "Disabling...");

    _enabled = false;

    ESP_LOGV(TAG, "Aquiring semaphore...");
	if (xSemaphoreTake(_bt_semaphore, (TickType_t)9999) == pdTRUE) {
        ESP_LOGV(TAG, "Deconfiguring esp_ble_mesg...");
        err = mesh_deconfigure_esp_ble_mesh();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to de-initialize NimBLE (%s)", esp_err_to_name(err));
        }

        ESP_LOGV(TAG, "De-initializing mesh host...");
        err = mesh_host_deinit();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to de-initialize NimBLE (%s)", esp_err_to_name(err));
        }

        xSemaphoreGive(_bt_semaphore);
    }

    ESP_LOGV(TAG, "De-initialize complete");
    _state = State::Disabled;

    return ESP_OK;
}

static TickType_t last_task_handler_at = 0;
#define TASK_HANDLER_INTERVAL 10

void BadgeMesh::taskHandler()
{
    esp_err_t err;
    TickType_t now = xTaskGetTickCount();
    TickType_t elapsed_ticks = now - last_task_handler_at;
    uint32_t elapsed_time_ms = (uint32_t)((elapsed_ticks * 1000) / configTICK_RATE_HZ);

    if(_state != State::Enabled)
        return;

    if(last_task_handler_at == 0 || elapsed_time_ms > (TASK_HANDLER_INTERVAL * 1000)) {
        if(!networkTimeValid)
            networkTimeRequest();

        // periodically persist the most recent sequence number
        mesh_sequence_number_changed();

        last_task_handler_at = now;
    }
}
