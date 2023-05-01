#pragma once

#include <esp_system.h>
#include <sys/queue.h>

#include "esp_ble_mesh_defs.h"
#include "mesh_buf.h"

#include "badge/mesh/config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OP_PARTYLINE                    0x08 /* must be unique */
#define OP_VND_PARTYLINE                ESP_BLE_MESH_MODEL_OP_3(OP_PARTYLINE, NSEC_COMPANY_ID)

#define PARTYLINE_MAX_MESSAGE_LENGTH 256
#define PARTYLINE_HISTORY_MAX_MESSAGES 20

typedef struct partyline_msg {
    time_t recv_at;
    uint16_t addr;
    char *from;
    char *msg;
} partyline_msg_t;

struct partyline_history_entry {
    partyline_msg_t data;
    STAILQ_ENTRY(partyline_history_entry) entries;        /* Singly linked tail queue */
};
STAILQ_HEAD(partyline_history, partyline_history_entry);
extern struct partyline_history partyline_history_head;

#define PARTYLINE_INSERT(elm)       STAILQ_INSERT_HEAD(&partyline_history_head, elm, entries)
#define PARTYLINE_LAST()            STAILQ_LAST(&partyline_history_head, partyline_history_entry, entries)
#define PARTYLINE_FOREACH(elm)      STAILQ_FOREACH(elm, &partyline_history_head, entries)
#define PARTYLINE_REMOVE(elm)       STAILQ_REMOVE(&partyline_history_head, elm, partyline_history_entry, entries)
#define PARTYLINE_FIRST()           STAILQ_FIRST(&partyline_history_head);

/* Iteration stops when callback returns false */
typedef bool (* partyline_each_cb_t)(partyline_msg_t *msg, void *param);

/* iterate all messages in order from most recently received to least recently received */
void partyline_each(partyline_each_cb_t cb, void *param);

/* Check if new messages have been received since a certain time, to avoid iterating needlessly */
bool partyline_received_since(time_t since);

/* init partyline history */
void partyline_history_init();

esp_err_t send_partyline(const char *msg);
esp_err_t partyline_received(esp_ble_mesh_model_t *model, esp_ble_mesh_msg_ctx_t *ctx, struct net_buf_simple *buf);

#ifdef __cplusplus
}
#endif
