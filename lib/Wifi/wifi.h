#ifndef WIFI_H
#define WIFI_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


struct Wifi
{
    bool mxInitialized, mxConnected;

    int miRetryCount;

    esp_netif_ip_info_t mIpInfo;
};
typedef struct Wifi Wifi;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static Wifi newWifi() {
    Wifi result;

    result.mxInitialized = false;
    result.mxConnected = false;

    return result;
}

void wifiInitClient(Wifi *pWifi, const char* wifiSSID, const char* wifiPwd);

void startWifi(Wifi *pWifi, const char* wifiSSID, const char* wifiPwd);


#endif  // WIFI_H