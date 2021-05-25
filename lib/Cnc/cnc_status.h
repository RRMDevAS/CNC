#ifndef CNC_STATUS_H
#define CNC_STATUS_H

#include "axis.h"

struct CncStatus {
    Axis maAxis[eAxisCount];
};
typedef struct CncStatus CncStatus;

struct AxisStatusMsg {
    float mfPosition;
};
typedef struct AxisStatusMsg AxisStatusMsg;

struct CncStatusMsg {
    AxisStatusMsg maAxis[eAxisCount];
};
typedef struct CncStatusMsg CncStatusMsg;

CncStatusMsg getStatusMsg(const CncStatus *status);

#endif      // CNC_STATUS_H