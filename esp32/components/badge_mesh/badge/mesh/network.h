#pragma once

#include <esp_system.h>

typedef struct badge_network_info {
    uint8_t  net_key[16];
    uint16_t net_idx;
    uint8_t  flags;
    uint32_t iv_index;
    uint16_t unicast_addr;
    uint8_t  dev_key[16];
    uint8_t  app_key[16];
    uint16_t app_idx;
    uint16_t group_addr;
} badge_network_info_t;

/*
    Check if device is properly provisionned as a node on a mesh network.
*/
int mesh_is_provisioned();

/*
    Self-provision the device to automatically enter a network.
*/
int mesh_device_auto_enter_network(badge_network_info_t *info);
