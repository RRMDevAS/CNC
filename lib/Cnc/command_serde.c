#include "command_serde.h"

#include "esp_log.h"

#include <string.h>

static const char *TAG = "SERDE";

int32_t deserializeCommand(CncCommand *cmd, int32_t byteCount, uint8_t *buffer) {
    if (byteCount<=0) {
        return -1;
    }
    switch (buffer[0])
    {
    case 0:
        if (byteCount!=13) {
            return -2;
        }
        float x, y, z;
        memcpy(&x, &buffer[1], sizeof(x));
        memcpy(&y, &buffer[5], sizeof(y));
        memcpy(&z, &buffer[9], sizeof(z));
        ESP_LOGI(TAG, "Received coords ( x: %f , y: %f , z: %f )", x, y, z);
        *cmd = newMoveCommand(0, x, y, z);
        return 1;
        break;
    case 1:
        ESP_LOGI(TAG, "Received quit");
        *cmd = newEndCommand(0);
        return 1;
    default:
        ESP_LOGI(TAG, "Unknown message received: %i", buffer[0]);
        return -3;
        break;
    }
    return -99;
}

int32_t serializeStatus(const CncStatusMsg* status, int32_t *byteCount, uint8_t *buffer) {
    
    buffer[0] = 0;

    memcpy(&buffer[1], &status->maAxis[eX].mfPosition, sizeof(status->maAxis[eX].mfPosition));
    memcpy(&buffer[5], &status->maAxis[eY].mfPosition, sizeof(status->maAxis[eY].mfPosition));
    memcpy(&buffer[9], &status->maAxis[eZ].mfPosition, sizeof(status->maAxis[eZ].mfPosition));

    *byteCount = 13;

    return 1;
}