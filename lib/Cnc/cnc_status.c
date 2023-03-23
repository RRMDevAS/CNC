#include "cnc_status.h"


const int64_t STATUS_UPDATE_RATE = 500000;

CncStatusMsg generateStatusMsg(const CncStatus *status, EStatusMsgType msgType) {
    CncStatusMsg res;

    res.meType = msgType;

    switch (msgType)
    {
    case eCoordinates:
        for (int i=0; i<eAxisCount; i++) {
            res.coords.mafX[i] = status->maAxis[i].mfPosition;
        }
        break;
    case eStatus:
        res.status.miCycleTime = status->miCycleTime;

        for (int i=0; i<eAxisCount; i++) {
            res.status.maAxis[i].mfPosition = status->maAxis[i].mfPosition;
            res.status.maAxis[i].mfSpeed = status->maAxis[i].mfSpeed;
            res.status.maAxis[i].mfTargetPosition = status->maAxis[i].mfTargetPosition;
            res.status.maAxis[i].mfTargetSpeed = status->maAxis[i].mfTargetSpeed;
            res.status.maAxis[i].mfPidPropControl = status->maAxis[i].mSpeedControl.mfPropControl;
            res.status.maAxis[i].mfPidIntControl = status->maAxis[i].mSpeedControl.mfIntControl;
            res.status.maAxis[i].mfPidDerControl = status->maAxis[i].mSpeedControl.mfDerControl;
            res.status.maAxis[i].miDuty = status->maAxis[i].miDuty;
        }
        break;
    case ePidParams:
        for (int i=0; i<eAxisCount; i++) {
            res.params.maParams[i].mfPropCoeff = status->maAxis[i].mSpeedControl.mParams.mfPropCoeff;
            res.params.maParams[i].mfIntCoeff = status->maAxis[i].mSpeedControl.mParams.mfIntCoeff;
            res.params.maParams[i].mfDerCoeff = status->maAxis[i].mSpeedControl.mParams.mfDerCoeff;
        }
        break;

    default:
        break;
    }

    return res;
}

bool getStatusMessage(CncStatus *cnc, CncStatusMsg* msg) {
    for (int32_t i=0; i<eStatusCount; i++) {
        if (cnc->maxTriggerUpdateMsg[i]) {
            *msg = generateStatusMsg( cnc, i);
            cnc->maxTriggerUpdateMsg[i] = false;
            return true;
        }
    }
    return false;
}

void updateStatus(CncStatus *cnc, int32_t iMSecs) {
    cnc->miStatusUpdate += iMSecs;
    if (cnc->miStatusUpdate>STATUS_UPDATE_RATE) {
        cnc->miStatusUpdate %= STATUS_UPDATE_RATE;
        cnc->maxTriggerUpdateMsg[eStatus] = true;
    }
}