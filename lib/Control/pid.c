#include "pid.h"



PidControl newPidControl(float propCoeff, float derCoeff, float intCoeff) {
    PidControl res = {
        .mfTarget           = 0.0f,
        .mfCurrent          = 0.0f,
        
        .mfError            = 0.0f, 
        .mfDerError         = 0.0f, 
        .mfIntError         = 0.0f,

        .mfPropCoeff        = propCoeff, 
        .mfDerCoeff         = derCoeff, 
        .mfIntCoeff         = intCoeff,
        
        .mfPropControl      = 0.0f, 
        .mfDerControl       = 0.0f, 
        .mfIntControl       = 0.0f,
        .mfControl          = 0.0f
    };

    return res;
}

void setPidTarget(PidControl* pid, float target) {
    pid->mfTarget = target;

    pid->mfError = 0.0f;
    pid->mfDerError = 0.0f;
    pid->mfIntError = 0.0f;
}

void setPidCurrent(PidControl* pid, float current, float timeDelta) {

    float fError = pid->mfTarget - current;

    if (timeDelta>0.0f) {
        pid->mfDerError = (fError - pid->mfError) / timeDelta;
        
        // approximate integral term error
        // reduces accumulated error by a tenth and adds new error
        pid->mfIntError = pid->mfIntError - pid->mfIntError * 0.1f * timeDelta + fError * timeDelta;
    }

    pid->mfError = fError;

    // calculate control terms
    calculatePidControl(pid);
}

void calculatePidControl(PidControl* pid) {
    pid->mfPropControl = pid->mfError * pid->mfPropCoeff;
    pid->mfDerControl = pid->mfDerError * pid->mfDerCoeff;
    pid->mfIntControl = pid->mfIntError * pid->mfIntCoeff;

    pid->mfControl = pid->mfPropControl + pid->mfDerControl + pid->mfIntControl;
}