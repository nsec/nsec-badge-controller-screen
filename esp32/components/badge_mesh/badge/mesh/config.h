#include <esp_system.h>

#include "esp_ble_mesh_defs.h"

#include "badge/mesh/network.h"

#define NSEC_COMPANY_ID         0xf00d

#define DEFAULT_TTL             31

#define NETWORK_GROUP_ADDR      0xc123
#define NETWORK_NET_KEY_IDX     0x000
#define NETWORK_APP_KEY_IDX     0x000
#define NETWORK_FLAGS           0

extern esp_ble_mesh_model_t *cli_vnd_model;
extern esp_ble_mesh_model_t *srv_vnd_model;

extern badge_network_info_t badge_network_info;

esp_err_t mesh_configure_esp_ble_mesh();
