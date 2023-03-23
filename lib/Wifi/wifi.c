#include "wifi.h"

#define MAXIMUM_RETRY  5

static const char *TAG = "WIFI";

static void eventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG, "################# Wifi event: %s | %i", event_base, event_id);

    Wifi *pWifi = (Wifi*)arg;
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            ESP_ERROR_CHECK(esp_wifi_connect());
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            wifi_event_sta_disconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
            ESP_LOGI(TAG, "Disconnected from %s ; L: %i; BSSID: %s - Reason %i", event->ssid, event->ssid_len, event->bssid, event->reason);

            pWifi->mxConnected = false;
            ESP_ERROR_CHECK(esp_wifi_connect());
            pWifi->miRetryCount++;
            ESP_LOGI(TAG, "Retry to connect to the AP: %i", pWifi->miRetryCount);
        } else {
            ESP_LOGI(TAG, "Unknown event: %s | %i", event_base, event_id);
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            pWifi->mIpInfo = event->ip_info;
            ESP_LOGI(TAG, "Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            pWifi->miRetryCount = 0;
            pWifi->mxConnected = true;
        } else {
            ESP_LOGI(TAG, "Unknown event: %s | %i", event_base, event_id);
        }
    } else {
        ESP_LOGI(TAG, "Unknown event: %s | %i", event_base, event_id);
    }
    // if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    //     ESP_ERROR_CHECK(esp_wifi_connect());
    // } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    //     wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
    //     ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);

    //     pWifi->mxConnected = false;
    //     ESP_ERROR_CHECK(esp_wifi_connect());
    //     pWifi->miRetryCount++;
    //     ESP_LOGI(TAG, "retry to connect to the AP: %i", pWifi->miRetryCount);
    // } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    //     ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    //     pWifi->mIpInfo = event->ip_info;
    //     ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    //     pWifi->miRetryCount = 0;
    //     pWifi->mxConnected = true;
    // } else {
    //     ESP_LOGI(TAG, "Unknown event: %s | %i", event_base, event_id);
    // }
}

void wifiInitClient(Wifi *pWifi, const char* wifiSSID, const char* wifiPwd)
{
    ESP_LOGI(TAG, "wifiInitClient starting.");
    // pWifi->mWifiEventGroup = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t* pEspNetIf = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // esp_event_handler_instance_t instance_any_id;
    // esp_event_handler_instance_t instance_got_ip;
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
    //                                                     ESP_EVENT_ANY_ID,
    //                                                     &eventHandler,
    //                                                     pWifi));
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
    //                                                     IP_EVENT_STA_GOT_IP,
    //                                                     &eventHandler,
    //                                                     pWifi));
    
    
    
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &eventHandler,
                                        pWifi,
                                        &pWifi->mInstanceAnyId));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &eventHandler,
                                        pWifi,
                                        &pWifi->mInstanceGotIp));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid               = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
            .password           = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
            .scan_method        = WIFI_ALL_CHANNEL_SCAN,
            .sort_method        = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi     = -127,
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .capable    = true,
                .required   = false
            },
        },
    };
    strcpy((char* )(wifi_config.sta.ssid), wifiSSID);
    strcpy((char* )(wifi_config.sta.password), wifiPwd);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifiInitClient finished.");

    pWifi->mxInitialized = true;
}

void startWifi(Wifi *pWifi, const char* wifiSSID, const char* wifiPwd)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_LOGI(TAG, "nvs_flash_erase");
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "startWifi");
    wifiInitClient(pWifi, wifiSSID, wifiPwd);
}