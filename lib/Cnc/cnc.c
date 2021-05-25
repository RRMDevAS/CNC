#include "cnc.h"

Cnc newCnc(AxisGpioPins xAxisPins, AxisGpioPins yAxisPins, AxisGpioPins zAxisPins) {
    Cnc res = {
        .mStatus = {
            .maAxis = {
                newAxis(eX, xAxisPins.gpioPwmA, xAxisPins.gpioPwmB, xAxisPins.gpioEncA, xAxisPins.gpioEncB),
                newAxis(eY, yAxisPins.gpioPwmA, yAxisPins.gpioPwmB, yAxisPins.gpioEncA, yAxisPins.gpioEncB),
                newAxis(eZ, zAxisPins.gpioPwmA, zAxisPins.gpioPwmB, zAxisPins.gpioEncA, zAxisPins.gpioEncB)
            }
        },
        .mCmd = newNoneCommand(0)
    };

    return res;
}

void initCnc(Cnc* cnc) {
    initializeAxis(&cnc->mStatus.maAxis[eX]);
    initializeAxis(&cnc->mStatus.maAxis[eY]);
    initializeAxis(&cnc->mStatus.maAxis[eZ]);
}

void updateCnc(Cnc* cnc, int32_t iMSecs) {
    axisEncoderUpdate(&cnc->mStatus.maAxis[eX], iMSecs);
    axisEncoderUpdate(&cnc->mStatus.maAxis[eY], iMSecs);
    axisEncoderUpdate(&cnc->mStatus.maAxis[eZ], iMSecs);

    updateCommand(&cnc->mCmd, &cnc->mStatus, iMSecs);

    axisMotorUpdate(&cnc->mStatus.maAxis[eX]);
    axisMotorUpdate(&cnc->mStatus.maAxis[eY]);
    axisMotorUpdate(&cnc->mStatus.maAxis[eZ]);
}