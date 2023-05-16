#include <esp_system.h>
#include <esp_log.h>
#include <esp_mac.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#include "save.h"
#include "wifi.h"

static const char *TAG = "wifi";
static esp_netif_t *netif_sta = NULL;

#define RAND_CHR (char)(esp_random() % 92 + 30)

static esp_event_handler_instance_t event_handler_instance;

static bool wifi_config_saved()
{
    return strlen(Save::save_data.wifi_ssid) && strlen(Save::save_data.wifi_password);
}

void Wifi::init()
{
    if(!wifi_config_saved()) {
        printf("generating random wifi config...\n");
        snprintf((char *)&Save::save_data.wifi_ssid, sizeof(Save::save_data.wifi_ssid), "PAD-%08lx", esp_random());
        snprintf((char *)&Save::save_data.wifi_password, sizeof(Save::save_data.wifi_password), "%c%c%c%c%c%c%c%c%c%c%c%c",
            RAND_CHR, RAND_CHR, RAND_CHR, RAND_CHR, RAND_CHR, RAND_CHR,
            RAND_CHR, RAND_CHR, RAND_CHR, RAND_CHR, RAND_CHR, RAND_CHR);
        printf("ssid=%s..\n", Save::save_data.wifi_ssid);
        printf("pwd=%s..\n", Save::save_data.wifi_password);
    }

    if(Save::save_data.debug_feature_enabled[debug_tab::wifi] && Save::save_data.debug_feature_enabled[debug_tab::mesh]) {
        Save::save_data.debug_feature_enabled[debug_tab::wifi] = false;
    }

    _enabled = false;
    _state = State::Disabled;

    if(Save::save_data.debug_feature_enabled[debug_tab::wifi]) {
        enable();
    } else {
        disable();
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

static void example_handler_on_sta_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    printf("Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
}

esp_err_t Wifi::wifiStaMode(void)
{
    wifi_config_t wifi_config = { };

     memcpy(wifi_config.sta.ssid, Save::save_data.wifi_ssid,
        sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, Save::save_data.wifi_password,
                               sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );
    esp_wifi_set_default_wifi_sta_handlers();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &example_handler_on_sta_got_ip, NULL));
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
    ESP_LOGE(TAG, "Wifi connected");

    return ESP_OK;
}

esp_err_t Wifi::enable()
{
    esp_err_t ret;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if(_enabled) {
        ESP_LOGV(TAG, "Already enabled...");
        return ESP_OK;
    }

    strcpy((char *)&_config.ap.ssid, Save::save_data.wifi_ssid);
    strcpy((char *)&_config.ap.password, Save::save_data.wifi_password);

    _config.ap.ssid_len = strlen((char *)&_config.ap.ssid);
    _config.ap.channel = 0;
    _config.ap.max_connection = 10;
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
    _config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    // _config.ap.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
#else
    _config.ap.authmode = WIFI_AUTH_WPA2_PSK;
#endif
    _config.ap.pmf_cfg.required = true;

    _enabled = true;

    ret = esp_netif_init();
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not initialize netif (%s)", __func__, esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ret = esp_event_loop_create_default();
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not create event loop (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }

    _netif_sta = esp_netif_create_default_wifi_ap();

    ret = esp_wifi_init(&cfg);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not initialize wifi (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }
    ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
        &wifi_event_handler, NULL, &event_handler_instance);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not register event handler (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }

    ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not set wifi AP mode (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }

    ret = esp_wifi_set_config(WIFI_IF_AP, &_config);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not set wifi config (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }

    ret = esp_wifi_start();
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not start wifi (%s)", __func__, esp_err_to_name(ret));
        goto fail;
    }
    assert(_netif_sta);

    // //ret = wifiStaMode();
    // //if(ret != ESP_OK) {
    //     ESP_LOGE(TAG, "%s: Could not start wifi STA mode (%s)", __func__, esp_err_to_name(ret));
    //     goto fail;
    // }

    _state = State::Enabled;

    return ESP_OK;
fail:
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler_instance);
    esp_wifi_deinit();
    esp_event_loop_delete_default();
    _state = State::Failed;
    return ret;
}

esp_err_t Wifi::disable()
{
    esp_err_t ret;

    if(!_enabled) {
        ESP_LOGV(TAG, "Already disabled...");
        return ESP_OK;
    }

    _enabled = false;

    ret = esp_wifi_stop();
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not stop wifi (%s)", __func__, esp_err_to_name(ret));
    }

    ret = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler_instance);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not unregister event handler instance (%s)", __func__, esp_err_to_name(ret));
    }

    ret = esp_wifi_deinit();
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not deinit wifi (%s)", __func__, esp_err_to_name(ret));
    }

    ret = esp_event_loop_delete_default();
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "%s: Could not delete default event loop (%s)", __func__, esp_err_to_name(ret));
    }

    _state = State::Disabled;

    return ESP_OK;
}
