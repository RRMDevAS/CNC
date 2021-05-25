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

// /* FreeRTOS event group to signal when we are connected*/
//     EventGroupHandle_t mWifiEventGroup;
};
typedef struct Wifi Wifi;

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
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