# Enable NimBLE
CONFIG_BT_ENABLED=y
CONFIG_BTDM_CTRL_MODE_BLE_ONLY=y
CONFIG_BTDM_CTRL_MODE_BR_EDR_ONLY=n
CONFIG_BTDM_CTRL_MODE_BTDM=n
CONFIG_BT_BLUEDROID_ENABLED=n
CONFIG_BT_NIMBLE_ENABLED=y

# Partition Table
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions_example_with_ble.csv"
CONFIG_PARTITION_TABLE_FILENAME="partitions_example_with_ble.csv"

CONFIG_EXAMPLE_CONNECT_WIFI=y
CONFIG_EXAMPLE_WIFI_SSID_PWD_FROM_STDIN=y
CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL="FROM_STDIN"
CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK=y
CONFIG_EXAMPLE_SKIP_VERSION_CHECK=y
CONFIG_EXAMPLE_OTA_RECV_TIMEOUT=3000

CONFIG_MBEDTLS_TLS_CLIENT_ONLY=y
CONFIG_COMPILER_OPTIMIZATION_SIZE=y
CONFIG_EXAMPLE_CONNECT_IPV6=n
