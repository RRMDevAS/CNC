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
    case eMove:
    {        
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
    }
    case ePidParamsUpdate: 
    {
        if (byteCount!=37) {
            return -2;
        }
        PidParams x, y, z;
        memcpy(&x, &buffer[1], sizeof(x));
        memcpy(&y, &buffer[13], sizeof(y));
        memcpy(&z, &buffer[25], sizeof(z));
        ESP_LOGI(TAG, "Received PID params ( x: (%f, %f, %f) , y: (%f, %f, %f) , z: (%f, %f, %f) )", 
                                            x.mfPropCoeff, x.mfIntCoeff, x.mfDerCoeff, 
                                            y.mfPropCoeff, y.mfIntCoeff, y.mfDerCoeff,
                                            z.mfPropCoeff, z.mfIntCoeff, z.mfDerCoeff);
        *cmd = newPidParamsCommand(0, &x, &y, &z);
        return 1;
        break;
    }
    case eEnd:
    {        
        ESP_LOGI(TAG, "Received quit");
        *cmd = newEndCommand(0);
        return 1;
        }
    case eSetSpeed:
    {        
        if (byteCount!=13) {
            return -2;
        }
        float x, y, z;
        memcpy(&x, &buffer[1], sizeof(x));
        memcpy(&y, &buffer[5], sizeof(y));
        memcpy(&z, &buffer[9], sizeof(z));
        ESP_LOGI(TAG, "Received speeds ( x: %f , y: %f , z: %f )", x, y, z);
        *cmd = newSetSpeedCommand(0, x, y, z);
        return 1;
        break;
        }
    default:
    {        
        ESP_LOGI(TAG, "Unknown message received: %i", buffer[0]);
        return -3;
        break;
        }
    }
    return -99;
}

int32_t serializeStatus(const CncStatusMsg* status, int32_t *byteCount, uint8_t *buffer) {
    
    buffer[0] = status->meType;

    int32_t iBufferOffset = 1;

    switch (status->meType)
    {
    case eCoordinates:
    {
        const CncCoordsMsgData* pCoords = &status->coords;

        for (uint32_t i=0; i<eAxisCount; i++) {
            memcpy(&buffer[iBufferOffset], &pCoords->mafX[i], sizeof(pCoords->mafX[i])); 
            iBufferOffset += sizeof(pCoords->mafX[i]);
        }
        break;
    }

    case eStatus:
    {
        const CncStatusMsgData* pSt = &status->status;
        memcpy(&buffer[iBufferOffset], &pSt->miCycleTime, sizeof(pSt->miCycleTime)); iBufferOffset += sizeof(pSt->miCycleTime);

        for (uint32_t i=0; i<eAxisCount; i++) {
            memcpy(&buffer[iBufferOffset], &pSt->maAxis[i].mfPosition, sizeof(pSt->maAxis[i].mfPosition)); 
            iBufferOffset += sizeof(pSt->maAxis[i].mfPosition);
            
            memcpy(&buffer[iBufferOffset], &pSt->maAxis[i].mfSpeed, sizeof(pSt->maAxis[i].mfSpeed)); 
            iBufferOffset += sizeof(pSt->maAxis[i].mfSpeed);

            memcpy(&buffer[iBufferOffset], &pSt->maAxis[i].mfTargetPosition, sizeof(pSt->maAxis[i].mfTargetPosition)); 
            iBufferOffset += sizeof(pSt->maAxis[i].mfTargetPosition);

            memcpy(&buffer[iBufferOffset], &pSt->maAxis[i].mfTargetSpeed, sizeof(pSt->maAxis[i].mfTargetSpeed)); 
            iBufferOffset += sizeof(pSt->maAxis[i].mfTargetSpeed);

            memcpy(&buffer[iBufferOffset], &pSt->maAxis[i].mfPidPropControl, sizeof(pSt->maAxis[i].mfPidPropControl)); 
            iBufferOffset += sizeof(pSt->maAxis[i].mfPidPropControl);

            memcpy(&buffer[iBufferOffset], &pSt->maAxis[i].mfPidIntControl, sizeof(pSt->maAxis[i].mfPidIntControl)); 
            iBufferOffset += sizeof(pSt->maAxis[i].mfPidIntControl);

            memcpy(&buffer[iBufferOffset], &pSt->maAxis[i].mfPidDerControl, sizeof(pSt->maAxis[i].mfPidDerControl)); 
            iBufferOffset += sizeof(pSt->maAxis[i].mfPidDerControl);

            memcpy(&buffer[iBufferOffset], &pSt->maAxis[i].miDuty, sizeof(pSt->maAxis[i].miDuty)); 
            iBufferOffset += sizeof(pSt->maAxis[i].miDuty);
        }
                    
        break;
    }

    case ePidParams: {
        const CncPidParamsMsgData* pParams = &status->params;

        for (uint32_t i=0; i<eAxisCount; i++) {
            memcpy(&buffer[iBufferOffset], &pParams->maParams[i].mfPropCoeff, sizeof(pParams->maParams[i].mfPropCoeff)); 
            iBufferOffset += sizeof(pParams->maParams[i].mfPropCoeff);
            
            memcpy(&buffer[iBufferOffset], &pParams->maParams[i].mfIntCoeff, sizeof(pParams->maParams[i].mfIntCoeff)); 
            iBufferOffset += sizeof(pParams->maParams[i].mfIntCoeff);

            memcpy(&buffer[iBufferOffset], &pParams->maParams[i].mfDerCoeff, sizeof(pParams->maParams[i].mfDerCoeff)); 
            iBufferOffset += sizeof(pParams->maParams[i].mfDerCoeff);
        }
                    
        break;
    }

    default:
        break;
    }

    *byteCount = iBufferOffset;

    return 1;
}