#ifndef PID_H
#define PID_H

struct PidControl {
    float mfTarget;
    float mfCurrent;

    float mfError, mfDerError, mfIntError;

    float mfPropCoeff, mfDerCoeff, mfIntCoeff;

    float mfPropControl, mfDerControl, mfIntControl;
    float mfControl;
};
typedef struct PidControl PidControl;

PidControl newPidControl(float propCoeff, float derCoeff, float intCoeff);

void setPidTarget(PidControl* pid, float target);
void setPidCurrent(PidControl* pid, float current, float timeDelta);

void calculatePidControl(PidControl* pid);

#endif  // PID_H