#ifndef CNC_H
#define CNC_H

#include "cnc_status.h"
#include "cnc_command.h"

struct Cnc {
    CncStatus mStatus;
    CncCommand mCmd;
};
typedef struct Cnc Cnc;

Cnc newCnc(AxisGpioPins xAxisPins, AxisGpioPins yAxisPins, AxisGpioPins zAxisPins);

void initCnc(Cnc* cnc);

void updateCnc(Cnc* cnc, int32_t iMSecs);

#endif      // CNC_H