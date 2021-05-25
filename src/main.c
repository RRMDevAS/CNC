/* CNC

    Author: A.Sljivo

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_task_wdt.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#include "encoder.h"
#include "axis.h"
#include "cnc.h"

#include "wifi.h"
#include "tcpserver.h"

#include "command_serde.h"

#define GPIO_X_PWM0A_OUT    18   //Set GPIO 15 as PWM0A
#define GPIO_X_PWM0B_OUT    23   //Set GPIO 16 as PWM0B
#define GPIO_X_ENC_A_IN     22
#define GPIO_X_ENC_B_IN     21

#define GPIO_Y_PWM0A_OUT    16   //Set GPIO 15 as PWM0A
#define GPIO_Y_PWM0B_OUT    17   //Set GPIO 16 as PWM0B
#define GPIO_Y_ENC_A_IN     32
#define GPIO_Y_ENC_B_IN     33

#define GPIO_Z_PWM0A_OUT    25   //Set GPIO 15 as PWM0A
#define GPIO_Z_PWM0B_OUT    26   //Set GPIO 16 as PWM0B
#define GPIO_Z_ENC_A_IN     27
#define GPIO_Z_ENC_B_IN     14

QueueHandle_t gCommandQueue;
QueueHandle_t gStatusQueue;

Wifi gWifi;
TcpServer gTcp;

Axis gAxis;

const int64_t STATUS_UPDATE_RATE = 200000;

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
    configCnc(&gCnc);

    int64_t iMSecs=0, iOldMSecs=0, iLogAcc = 0, iDeltaMSecs=0, iMotorDir = 0;
    int64_t iStatusUpdate = 0;
    int64_t iDelaySecs = 2;
    bool xReverse = false;
    UBaseType_t uxHighWaterMark;
    while (1)
    {
        if (!gCnc.mStatus.maAxis[eX].mxInitialized) continue;
        if (!gCnc.mStatus.maAxis[eY].mxInitialized) continue;
        if (!gCnc.mStatus.maAxis[eZ].mxInitialized) continue;

        iMSecs = esp_timer_get_time();
        iDeltaMSecs = iMSecs - iOldMSecs;
        iOldMSecs = iMSecs;

        CncCommand recvCmd;
        if (xQueueReceive(gCommandQueue, (void*)&recvCmd, (TickType_t)0)) {
            gCnc.mCmd = recvCmd;
        }

        updateCnc(&gCnc, iDeltaMSecs);

        iStatusUpdate += iDeltaMSecs;
        if (iStatusUpdate>=STATUS_UPDATE_RATE) {
            iStatusUpdate -= STATUS_UPDATE_RATE;
            CncStatusMsg msg = getStatusMsg(&gCnc.mStatus);
            // msg.maAxis[eX].mfPosition = 51.0;
            // msg.maAxis[eY].mfPosition = 22.0;
            // msg.maAxis[eZ].mfPosition = 33.0;
            xQueueSend(gStatusQueue, (void*)&msg, (TickType_t)0);
        }

        // // axisEncoderUpdate(&gAxis, (float)iDeltaMSecs * 0.000001f);
        // axisEncoderUpdate(&gAxis, iDeltaMSecs);
        // axisMotorUpdate(&gAxis);
        // // vTaskDelay(1000 / portTICK_RATE_MS);

        iLogAcc += iDeltaMSecs;
        if (iLogAcc/1000000>=0) {
            iLogAcc -= 1000000;

            if (fabs(gAxis.mfPosition-gAxis.mfTargetPosition)<1.f) {
            }
            uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
            ESP_LOGI(TAG, "axis task stack size: %i", uxHighWaterMark);
        }
    }
}

/**
 * @brief Configure MCPWM module for brushed dc motor
 */
static void initAxis()
{

    gAxis = newAxis(eX, GPIO_X_PWM0A_OUT, GPIO_X_PWM0B_OUT, GPIO_X_ENC_A_IN, GPIO_X_ENC_B_IN);
    initializeAxis(&gAxis);

    int64_t iMSecs=0, iOldMSecs=0, iLogAcc = 0, iDeltaMSecs=0, iMotorStartDelay = 10;

    // while (1)
    // {
    //     // // if (fabs(gAxis.mfPosition-gAxis.mfTargetPosition)<0.1f) {
    //     // //     vTaskDelay(1000 / portTICK_RATE_MS);
    //     // //     if (gAxis.mfTargetPosition==0.f) {
    //     // //         gAxis.mfTargetPosition = 10.f;
    //     // //     } else {
    //     // //         gAxis.mfTargetPosition = 0.f;
    //     // //     }
    //     // // }
    //     // iMSecs = esp_timer_get_time();
    //     // iDeltaMSecs = iMSecs - iOldMSecs;
    //     // axisEncoderUpdate(&gAxis, (float)iDeltaMSecs * 0.000001f);
    //     // iOldMSecs = iMSecs;
    //     // axisMotorUpdate(&gAxis);
    //     // // vTaskDelay(1000 / portTICK_RATE_MS);

    //     // iLogAcc += iDeltaMSecs;
    //     // if (iLogAcc>=1000000) {
    //     //     printAxis(&gAxis);
    //     //     iLogAcc -= 1000000;
    //     //     printf("elapsed %lld\n", iMSecs);
    //     //     // printf("encoder %lld\n", getCount(&gAxis.mEncoder) );
    //     //     if (iMotorStartDelay<=0) {
    //     //         if (gAxis.mfTargetPosition==0.f) {
    //     //             gAxis.mfTargetPosition = -1000.f;
    //     //         } else {
    //     //             gAxis.mfTargetPosition = 0.f;
    //     //         }
    //     //         iMotorStartDelay = 10;
    //     //     }
    //     //     iMotorStartDelay--;
    //     // }
    //     // // vTaskDelay(1000 / portTICK_RATE_MS);

    //     // // esp_task_wdt_reset();
    // }
    
}

static void taskAxisMotorControl(void *arg) {
    int64_t iMSecs=0, iOldMSecs=0, iLogAcc = 0, iDeltaMSecs=0, iMotorDir = 0;
    int64_t iDelaySecs = 2;
    bool xReverse = false;
    UBaseType_t uxHighWaterMark;
    while (1)
    {
        if (!gAxis.mxInitialized) continue;

        iMSecs = esp_timer_get_time();
        iDeltaMSecs = iMSecs - iOldMSecs;
        iOldMSecs = iMSecs;
        // axisEncoderUpdate(&gAxis, (float)iDeltaMSecs * 0.000001f);
        axisEncoderUpdate(&gAxis, iDeltaMSecs);
        axisMotorUpdate(&gAxis);
        // vTaskDelay(1000 / portTICK_RATE_MS);

        iLogAcc += iDeltaMSecs;
        if (iLogAcc/1000000>=0) {
            // printAxis(&gAxis);
            iLogAcc -= 1000000;

            if (fabs(gAxis.mfPosition-gAxis.mfTargetPosition)<1.f) {
                iMotorDir--;
                // vTaskDelay(5000 / portTICK_RATE_MS);
                if (iMotorDir<=0) {
                    if (gAxis.mfTargetPosition==0.f) {
                        gAxis.mfTargetPosition = 30.f;
                    } else {
                        gAxis.mfTargetPosition = 0.f;
                    }
                    iMotorDir = 5;
                }
            }
            // uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
            // ESP_LOGI(TAG, "axis task stack size: %i", uxHighWaterMark);
        }
    }
}

static void taskNetwork(void *arg) {

    int64_t iMSecs=0, iOldMSecs=0, iLogAcc = 0, iDeltaMSecs=0, iMotorDir = 0;
    int64_t iDelaySecs = 2;

    uint8_t abReceivedData[512];
    uint32_t iReceivedCount;

    gWifi = newWifi();
    startWifi(&gWifi, "NET", "PWD");
    vTaskDelay(5000 / portTICK_RATE_MS);

    char astrAddress[128];

    UBaseType_t uxHighWaterMark;

    while (1)
    {

        iMSecs = esp_timer_get_time();
        iDeltaMSecs = iMSecs - iOldMSecs;
        iOldMSecs = iMSecs;
        iLogAcc += iDeltaMSecs;

        if (!gWifi.mxConnected) {
            if (iLogAcc/1000000>=0) {
                iLogAcc -= 1000000;
                ESP_LOGI(TAG, "Wifi not connected");
                // uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
                // ESP_LOGI(TAG, "network task stack size: %i", uxHighWaterMark);
            }
            continue;
        }

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
            if (iLogAcc/1000000>=0) {
                // ESP_LOGI(TAG, "going to accept a connection");
                acceptConnection(&gTcp);
            }
            break;
        case eConnected:
            receiveData(&gTcp);
            if (!sendData(&gTcp) && gTcp.mClient.mSendBuffer.muSize>0) {
                ESP_LOGI(TAG, "this is not sending");
            }
            break;
        default:
            ESP_LOGI(TAG, "Unknown tcp server state %i", gTcp.meState);
            break;
        }
        pullFromBuffer(&gTcp.mClient.mReceiveBuffer, &iReceivedCount, abReceivedData);
        if (iReceivedCount>0) {
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

        if (iLogAcc/1000000>=0) {
            iLogAcc -= 1000000;

            printTcpStatus(&gTcp);

            iDelaySecs--;
            if (iDelaySecs<=0) {
                iDelaySecs = 10;
            }
        }
    }
    shutdownServer(&gTcp);
}

void app_main(void)
{
    printf("Testing brushed motor...\n");
    setupQueues();
    // configCnc();
    // xTaskCreate(axisControl, "axisControl", 4096, NULL, 2, NULL);
    // xTaskCreate(taskAxisMotorControl, "taskAxisMotorControl", 8192, NULL, 3, NULL);
    xTaskCreate(taskNetwork, "taskNetwork", 8192, NULL, 3, NULL);
    xTaskCreate(taskCncControl, "taskCnc", 8192, NULL, 3, NULL);
}
