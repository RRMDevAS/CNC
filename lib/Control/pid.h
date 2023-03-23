#ifndef PID_H
#define PID_H

struct PidParams {
    float mfPropCoeff, mfIntCoeff, mfDerCoeff;
};
typedef struct PidParams PidParams;

struct PidControl {
    float mfTarget;
    float mfCurrent;

    float mfError, mfIntError, mfDerError;
    
    PidParams mParams;

    float mfPropControl, mfIntControl, mfDerControl;
    float mfControl;
};
typedef struct PidControl PidControl;

PidControl newPidControl(float propCoeff, float derCoeff, float intCoeff);

void setPidTarget(PidControl* pid, float target);
void setPidCurrent(PidControl* pid, float current, float timeDelta);

void calculatePidControl(PidControl* pid);

#endif  // PID_H