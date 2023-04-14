#include <string.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_err.h>

#ifdef CONFIG_BT_NIMBLE_ENABLED
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#endif

#include "badge/mesh/host.h"

static const char *TAG = "badge/mesh";

static SemaphoreHandle_t mesh_sem;

uint8_t _device_address_type;
uint8_t _device_address[6] = {0};
bool mesh_host_initialized = false;

static void mesh_on_reset(int reason)
{
    ESP_LOGI(TAG, "Resetting state; reason=%d", reason);
}

static void mesh_on_sync(void)
{
    int rc;

    ESP_LOGV(TAG, "%s: Bluetooth initialized", __func__);

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &_device_address_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "error determining address type; rc=%d", rc);
        goto cleanup;
    }

    rc = ble_hs_id_copy_addr(_device_address_type, _device_address, NULL);
	if (rc) {
		ESP_LOGE(TAG, "Failed to get own address; rc=%d", rc);
		goto cleanup;
	}

    mesh_host_initialized = true;

cleanup:
    xSemaphoreGive(mesh_sem);
}

static void mesh_host_task(void *param)
{
    ESP_LOGV(TAG, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

esp_err_t mesh_host_initialize(void)
{
    mesh_sem = xSemaphoreCreateBinary();
    if (mesh_sem == NULL) {
        ESP_LOGE(TAG, "Failed to create mesh semaphore");
        return ESP_FAIL;
    }

    nimble_port_init();
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = mesh_on_reset;
    ble_hs_cfg.sync_cb = mesh_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    nimble_port_freertos_init(mesh_host_task);

    xSemaphoreTake(mesh_sem, portMAX_DELAY);

    return ESP_OK;
}
