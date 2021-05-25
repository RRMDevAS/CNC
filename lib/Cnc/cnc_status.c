#include "cnc_status.h"

CncStatusMsg getStatusMsg(const CncStatus *status) {
    CncStatusMsg res;

    for (int i=0; i<eAxisCount; i++) {
        res.maAxis[i].mfPosition = status->maAxis[i].mfPosition;
    }

    return res;
}