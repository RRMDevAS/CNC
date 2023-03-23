#include "cnc.h"

Cnc newCnc(AxisGpioPins xAxisPins, AxisGpioPins yAxisPins, AxisGpioPins zAxisPins) {
    Cnc res = {
        .mStatus = {
            .miCycleTime = 0,
            .miStatusUpdate = 0,
            .maAxis = {
                newAxis(eX, xAxisPins.gpioPwmA, xAxisPins.gpioPwmB, xAxisPins.gpioEncA, xAxisPins.gpioEncB),
                newAxis(eY, yAxisPins.gpioPwmA, yAxisPins.gpioPwmB, yAxisPins.gpioEncA, yAxisPins.gpioEncB),
                newAxis(eZ, zAxisPins.gpioPwmA, zAxisPins.gpioPwmB, zAxisPins.gpioEncA, zAxisPins.gpioEncB)
            },
            .maxTriggerUpdateMsg = { false, false, true },
        },
        .mCmd = newNoneCommand(0)
    };

    setAxisLead(&res.mStatus.maAxis[eX], 8.f);
    setAxisLead(&res.mStatus.maAxis[eY], 8.f);
    setAxisLead(&res.mStatus.maAxis[eZ], 2.f);

    return res;
}

void initCnc(Cnc* cnc) {
    initializeAxis(&cnc->mStatus.maAxis[eX]);
    initializeAxis(&cnc->mStatus.maAxis[eY]);
    initializeAxis(&cnc->mStatus.maAxis[eZ]);
}

void updateCnc(Cnc* cnc, int32_t iMSecs) {
    updateStatus(&cnc->mStatus, iMSecs);

    if (cnc->mStatus.maAxis[eX].mxInitialized) axisEncoderUpdate(&cnc->mStatus.maAxis[eX], iMSecs);
    if (cnc->mStatus.maAxis[eY].mxInitialized) axisEncoderUpdate(&cnc->mStatus.maAxis[eY], iMSecs);
    if (cnc->mStatus.maAxis[eZ].mxInitialized) axisEncoderUpdate(&cnc->mStatus.maAxis[eZ], iMSecs);

    updateCommand(&cnc->mCmd, &cnc->mStatus, iMSecs);

    if (cnc->mStatus.maAxis[eX].mxInitialized) axisMotorUpdate(&cnc->mStatus.maAxis[eX]);
    if (cnc->mStatus.maAxis[eY].mxInitialized) axisMotorUpdate(&cnc->mStatus.maAxis[eY]);
    if (cnc->mStatus.maAxis[eZ].mxInitialized) axisMotorUpdate(&cnc->mStatus.maAxis[eZ]);

    cnc->mStatus.miCycleTime = iMSecs;
}

