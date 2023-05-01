#include <string.h>
#include <esp_system.h>
#include <esp_log.h>

#include "esp_ble_mesh_defs.h"

#include "display.h"
#include "badge/mesh/host.h"
#include "badge/mesh/main.h"
#include "badge/mesh/ops/partyline.h"

static const char *TAG = "badge/mesh";

static unsigned int partyline_history_length = 0;
static SemaphoreHandle_t _partyline_semaphore = NULL;
struct partyline_history partyline_history_head;

void partyline_history_init()
{
    STAILQ_INIT(&partyline_history_head);
    _partyline_semaphore = xSemaphoreCreateMutex();
}

static void partyline_history_insert_head(uint16_t addr, char *from, char *msg)
{
    struct partyline_history_entry *entry;

    entry = (struct partyline_history_entry *)malloc(sizeof(struct partyline_history_entry));
    PARTYLINE_INSERT(entry);
    BadgeMesh::getInstance().networkTimeGet(&entry->data.recv_at);
    entry->data.addr = addr;
    entry->data.from = strdup(from);
    entry->data.msg = strdup(msg);

    if(partyline_history_length == PARTYLINE_HISTORY_MAX_MESSAGES) {
        entry = PARTYLINE_LAST();
        if(entry) {
            PARTYLINE_REMOVE(entry);
            free(entry->data.from);
            free(entry->data.msg);
            free(entry);
        }
    } else {
        partyline_history_length++;
    }
}

bool partyline_received_since(time_t since)
{
    struct partyline_history_entry *entry;

    entry = PARTYLINE_FIRST();
    return entry && entry->data.recv_at > since;
}

void partyline_each(partyline_each_cb_t cb, void *param)
{
    struct partyline_history_entry *np;

	if (xSemaphoreTake(_partyline_semaphore, (TickType_t)9999) == pdTRUE) {
        PARTYLINE_FOREACH(np) {
            if(!cb(&np->data, param)) {
                break;
            }
        }

        xSemaphoreGive(_partyline_semaphore);
    }

    return;
}

esp_err_t send_partyline(const char *msg)
{
    esp_err_t err;
    char *from = badge_network_info.name;
    int from_len = strlen(from);
    int msg_len = strlen(msg);

    if(msg_len > PARTYLINE_MAX_MESSAGE_LENGTH)
        return ESP_FAIL;

    int data_len = from_len + msg_len + 2;
    char *data = (char *)malloc(data_len);

    strcpy(data, from);
    strcpy(&data[from_len+1], msg);

    ESP_LOGV(TAG, "%s: from='%s' msg='%s'", __func__, from, msg);

    err = mesh_client_send(NETWORK_GROUP_ADDR, OP_VND_PARTYLINE, (uint8_t *)data, data_len, false);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: failed", __func__);
        return err;
    }

    return ESP_OK;
}

esp_err_t partyline_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf)
{
    char *from = NULL;
    char *msg = NULL;
	// if (ctx->addr == model->element->element_addr) {
    //     ESP_LOGV(TAG, "%s: Ignoring message from self", __func__);
    //     return ESP_OK;
    // }

    if(buf->data[buf->len - 1] != '\0') {
        ESP_LOGV(TAG, "Partyline message is not zero terminated from node=0x%04x", ctx->addr);
        return ESP_FAIL; /* not zero terminated */
    }

    if(strlen((const char *)buf->data) <= 0) {
        ESP_LOGV(TAG, "Partyline message too short to contain a name from node=0x%04x", ctx->addr);
        return ESP_FAIL; /* not zero terminated */
    }

    from = (char *)buf->data;

    if(buf->len <= strlen((const char *)buf->data) + 2) {
        ESP_LOGV(TAG, "Partyline message too short to contain a message from node=0x%04x", ctx->addr);
        return ESP_FAIL; /* not zero terminated */
    }

    msg = (char *)&buf->data[strlen((const char *)buf->data)+1];
    if(strlen(msg) > PARTYLINE_MAX_MESSAGE_LENGTH) {
        ESP_LOGV(TAG, "Partyline message too long from node=0x%04x", ctx->addr);
        return ESP_FAIL;
    }

    ESP_LOGV(TAG, "Received partyline message from node=0x%04x from='%s' msg='%s'", ctx->addr, from, msg);

	if (xSemaphoreTake(_partyline_semaphore, (TickType_t)10) == pdTRUE) {
        partyline_history_insert_head(ctx->addr, from, msg);

        xSemaphoreGive(_partyline_semaphore);
    }

    return ESP_OK;
}
