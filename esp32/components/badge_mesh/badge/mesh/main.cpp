#include <esp_system.h>
#include <esp_log.h>

#include "badge/mesh/main.h"
#include "badge/mesh/host.h"
#include "badge/mesh/config.h"

#include "badge/mesh/ops/ping.h"

static const char *TAG = "badge/mesh";

TaskHandle_t BadgeMesh::_taskHandle = NULL;

uint64_t _current_ping_idx = 0;

void BadgeMesh::init()
{
    xTaskCreate((TaskFunction_t)&(BadgeMesh::task), TAG, 4096, this, 5, &BadgeMesh::_taskHandle);
}

void BadgeMesh::taskHandler()
{
    esp_err_t err;

    vTaskDelay(1500 / portTICK_PERIOD_MS);

    ESP_LOGV(TAG, "Initializing...");

    err = mesh_host_initialize();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NimBLE");
        return;
    }

    while(!mesh_host_initialized)
    {
        // Wait for the async mesh initialization process to finish
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

	ESP_LOGV(TAG, "Mesh host has been initialized");

	err = mesh_configure_esp_ble_mesh();
	if (err) {
		ESP_LOGE(TAG, "Initializing mesh failed (err %d)", err);
		return;
	}

	if (!mesh_is_provisioned()) {
		ESP_LOGE(TAG, "This node is not properly initialized to be part of a mesh");
        return;
	} else {
	    ESP_LOGV(TAG, "This node is provisioned correctly");
	}

    while(true) {
        // send_ping(_current_ping_idx++);

        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(BadgeMesh::_taskHandle);
}
