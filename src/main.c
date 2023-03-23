/* CNC

    Author: A.Sljivo

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "encoder.h"
#include "axis.h"
#include "cnc.h"

#include "wifi.h"
#include "tcpserver.h"

#include "command_serde.h"

#include "timer_bits.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_task_wdt.h"

#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "soc/mcpwm_periph.h"

#include <stdio.h>
#include <string.h>
#include <math.h>


// GPIO
#define GPIO_X_PWM0A_OUT    32   //Set GPIO 32 as PWM0A
#define GPIO_X_PWM0B_OUT    33   //Set GPIO 33 as PWM0B
#define GPIO_X_ENC_A_IN     34
#define GPIO_X_ENC_B_IN     35

#define GPIO_Y_PWM0A_OUT    25   //Set GPIO 15 as PWM0A
#define GPIO_Y_PWM0B_OUT    26   //Set GPIO 16 as PWM0B
#define GPIO_Y_ENC_A_IN     27
#define GPIO_Y_ENC_B_IN     15

#define GPIO_Z_PWM0A_OUT    23   //Set GPIO 15 as PWM0A
#define GPIO_Z_PWM0B_OUT    18   //Set GPIO 16 as PWM0B
#define GPIO_Z_ENC_A_IN     04
#define GPIO_Z_ENC_B_IN     21

#define GPIO_ONBOARD_LED    2

QueueHandle_t gCommandQueue;
QueueHandle_t gStatusQueue;

Wifi gWifi;
TcpServer gTcp;

Axis gAxis;

// const int64_t STATUS_UPDATE_RATE = 500000;
const int64_t DELAY_INIT = 20000000;

static const char *TAG = "MAIN";

static void setupQueues() {
    gCommandQueue = xQueueCreate(1, sizeof(CncCommand));
    gStatusQueue = xQueueCreate(1, sizeof(CncStatusMsg));
}

static void configCnc(Cnc *cnc) {
    AxisGpioPins xAxisPins = {
        .gpioPwmA = GPIO_X_PWM0A_OUT,
        .gpioPwmB = GPIO_X_PWM0B_OUT,
        .gpioEncA = GPIO_X_ENC_A_IN,
        .gpioEncB = GPIO_X_ENC_B_IN
    };
    AxisGpioPins yAxisPins = {
        .gpioPwmA = GPIO_Y_PWM0A_OUT,
        .gpioPwmB = GPIO_Y_PWM0B_OUT,
        .gpioEncA = GPIO_Y_ENC_A_IN,
        .gpioEncB = GPIO_Y_ENC_B_IN
    };
    AxisGpioPins zAxisPins = {
        .gpioPwmA = GPIO_Z_PWM0A_OUT,
        .gpioPwmB = GPIO_Z_PWM0B_OUT,
        .gpioEncA = GPIO_Z_ENC_A_IN,
        .gpioEncB = GPIO_Z_ENC_B_IN
    };

    *cnc = newCnc(xAxisPins, yAxisPins, zAxisPins);
    initCnc(cnc);
}

static void taskCncControl(void *arg) {
    Cnc gCnc;

    CncCommand recvCmd;     // command received through TCP

    int64_t iMSecs=0, iOldMSecs=0, iLogAcc = 0, iDeltaMSecs=0;   // time
    int64_t iStatusUpdate = 0;

    int64_t iDelayInit = 0;
    bool xInit = false;
    
    UBaseType_t uxHighWaterMark;

    TimerBits timerBts = newTimerBits();

    CncStatusMsg msg;

    while (1)
    {
        iMSecs = esp_timer_get_time();
        iDeltaMSecs = iMSecs - iOldMSecs;
        iOldMSecs = iMSecs;

        updateTimerBits(&timerBts, iDeltaMSecs);

        if (!xInit) {
            iDelayInit += iDeltaMSecs;
            if (iDelayInit>=DELAY_INIT) {
                configCnc(&gCnc);
                xInit = true;
                ESP_LOGI(TAG, "CNC initialized");
            }
            continue;
        }

        if (!gCnc.mStatus.maAxis[eX].mxInitialized) {
            if (timerBts.mxImpuls_5s) {
                ESP_LOGI(TAG, "axis X not initialized");
            }
            // continue;
        }
        if (!gCnc.mStatus.maAxis[eY].mxInitialized) {
            if (timerBts.mxImpuls_5s) {
                ESP_LOGI(TAG, "axis Y not initialized");
            }
            // continue;
        }
        if (!gCnc.mStatus.maAxis[eZ].mxInitialized) {
            if (timerBts.mxImpuls_5s) {
                ESP_LOGI(TAG, "axis Z not initialized");
            }
            // continue;
        }

        if (xQueueReceive(gCommandQueue, (void*)&recvCmd, (TickType_t)0)) {
            gCnc.mCmd = recvCmd;
        }

        updateCnc(&gCnc, iDeltaMSecs);

        // iStatusUpdate += iDeltaMSecs;
        // if (iStatusUpdate>=STATUS_UPDATE_RATE) {
        //     // ESP_LOGI(TAG, "Status update");
        //     iStatusUpdate -= STATUS_UPDATE_RATE;
        //     CncStatusMsg msg = getStatusMsg(&gCnc.mStatus, eStatus);
        //     // msg.maAxis[eX].mfPosition = 51.0;
        //     // msg.maAxis[eY].mfPosition = 22.0;
        //     // msg.maAxis[eZ].mfPosition = 33.0;
        //     xQueueSend(gStatusQueue, (void*)&msg, (TickType_t)0);
        // }
        
        if ( getStatusMessage(&gCnc, &msg)) {
            xQueueSend(gStatusQueue, (void*)&msg, (TickType_t)0);
        }

        // // axisEncoderUpdate(&gAxis, (float)iDeltaMSecs * 0.000001f);
        // axisEncoderUpdate(&gAxis, iDeltaMSecs);
        // axisMotorUpdate(&gAxis);
        // // vTaskDelay(1000 / portTICK_RATE_MS);

        iLogAcc += iDeltaMSecs;
        if (iLogAcc/10000000>=0) {
            iLogAcc -= 10000000;
            
            uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
            ESP_LOGI(TAG, "axis task stack size: %i, cycle time: %i", uxHighWaterMark, gCnc.mStatus.miCycleTime);
        }
    }
}


static void taskNetwork(void *arg) {

    int64_t iMSecs=0, iOldMSecs=0, iLogAcc = 0, iDeltaMSecs=0, iMotorDir = 0;
    int64_t iDelaySecs = 2;

    uint8_t abReceivedData[512];
    uint32_t iReceivedCount;

    // blue led indication
    bool xBlueLed = false;

    gpio_pad_select_gpio(GPIO_ONBOARD_LED);
    gpio_set_direction (GPIO_ONBOARD_LED,GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_ONBOARD_LED, 0);

    // wifi
    gWifi = newWifi();
    startWifi(&gWifi, "WIFI-SSID", "WIFI-PWD");
    vTaskDelay(5000 / portTICK_RATE_MS);

    char astrAddress[128];

    UBaseType_t uxHighWaterMark;

    TimerBits timerBts = newTimerBits();

    while (1)
    {

        iMSecs = esp_timer_get_time();
        iDeltaMSecs = iMSecs - iOldMSecs;
        iOldMSecs = iMSecs;
        iLogAcc += iDeltaMSecs;

        updateTimerBits(&timerBts, iDeltaMSecs);

        xBlueLed = timerBts.mxTact_0_2s;

        if (!gWifi.mxConnected) {
            if (iLogAcc>=5000000) {
                iLogAcc %= 5000000;
                ESP_LOGI(TAG, "Wifi not connected");
                // uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
                // ESP_LOGI(TAG, "network task stack size: %i", uxHighWaterMark);
            }
            // blue led
            if (xBlueLed) {
                gpio_set_level(GPIO_ONBOARD_LED, 1);
            } else {
                gpio_set_level(GPIO_ONBOARD_LED, 0);
            }

            //
            continue;
        }

        xBlueLed = timerBts.mxTact_2s;

        switch (gTcp.meState)
        {
        case eNew:
            sprintf(astrAddress, IPSTR, IP2STR(&gWifi.mIpInfo.ip));
            gTcp = newTcpServer(astrAddress, 5555);
            initializeTcpServer(&gTcp);
            break;
        case eInitialized:
            break;
        case eSocketBound:
        case eListening:
            if (iLogAcc/10000000>=0) {
                // ESP_LOGI(TAG, "going to accept a connection");
                acceptConnection(&gTcp);
            }
            break;
        case eConnected:
            receiveData(&gTcp);
            if (!sendData(&gTcp) && gTcp.mClient.mSendBuffer.muSize>0) {
                ESP_LOGI(TAG, "Failed to send data");
            }
            break;
        default:
            ESP_LOGI(TAG, "Unknown tcp server state %i", gTcp.meState);
            break;
        }
        pullFromBuffer(&gTcp.mClient.mReceiveBuffer, &iReceivedCount, abReceivedData);
        if (iReceivedCount>0) {
            xBlueLed = false;
            ESP_LOGI(TAG, "Rx: %s", (const char*)abReceivedData);
            CncCommand cmd;
            if (deserializeCommand(&cmd, iReceivedCount, abReceivedData)>=0) {
                xQueueSend(gCommandQueue, (void*)&cmd, 0);
            }
        }
        CncStatusMsg sts;
        if (xQueueReceive(gStatusQueue, (void*)&sts, 0)) {
            int32_t iBytes;
            uint8_t abBuffer[sizeof(CncStatusMsg)];
            if (serializeStatus(&sts, &iBytes, abBuffer)>=0) {
                pushToBuffer(&gTcp.mClient.mSendBuffer, iBytes, abBuffer);
            }
        }

        if (iLogAcc>=1000000) {
            iLogAcc %= 1000000;


            iDelaySecs--;
            if (iDelaySecs<=0) {
                iDelaySecs = 10;
                printTcpStatus(&gTcp);
            }
        }

        if (xBlueLed) {
            gpio_set_level(GPIO_ONBOARD_LED, 1);
        } else {
            gpio_set_level(GPIO_ONBOARD_LED, 0);
        }
    }
    shutdownServer(&gTcp);
}

void app_main(void)
{
    setupQueues();
    
    BaseType_t iTaskNetRes = xTaskCreatePinnedToCore(taskNetwork, "NET", 8192, NULL, 3, NULL, 0);
    BaseType_t iTaskCncRes = xTaskCreatePinnedToCore(taskCncControl, "CNC", 8192, NULL, 3, NULL, 1);

    ESP_LOGI(TAG, "Net task res: %i", iTaskNetRes);
    ESP_LOGI(TAG, "Cnc task res: %i", iTaskCncRes);
}
