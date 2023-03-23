#ifndef DCMOTOR_H
#define DCMOTOR_H

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#include "esp_log.h"

struct Axle {
    float mfAngle;
    float mfOmega;
};
typedef struct Axle Axle;


struct DCMotor {
    mcpwm_unit_t miMCPUnit;
    int miSlot;
    mcpwm_io_signals_t mePwmSignalA, mePwmSignalB;
    mcpwm_timer_t meTimer;
    int miGpioPwmA, miGpioPwmB;
    float mfDuty;
    bool mxInitialized;
};
typedef struct DCMotor DCMotor;

static DCMotor newMotor(mcpwm_unit_t unitId, 
                        int slot,
                        int gpioPwmA, int gpioPwmB) {
    mcpwm_io_signals_t ePwmA, ePwmB;
    mcpwm_timer_t eTimer;
    if (slot==0) {
        ePwmA = MCPWM0A;
        ePwmB = MCPWM0B;
        eTimer = MCPWM_TIMER_0;
    } else if (slot==1) {
        ePwmA = MCPWM1A;
        ePwmB = MCPWM1B;
        eTimer = MCPWM_TIMER_1;
    } else if (slot==2) {
        ePwmA = MCPWM2A;
        ePwmB = MCPWM2B;
        eTimer = MCPWM_TIMER_2;
    } else {
        ePwmA = MCPWM0A;
        ePwmB = MCPWM0B;
        eTimer = MCPWM_TIMER_0;
    }

    DCMotor encDcMotor = { 
        .miMCPUnit = unitId,
        .miSlot = slot, 
        .mePwmSignalA = ePwmA,
        .mePwmSignalB = ePwmB,
        .meTimer = eTimer, 
        .miGpioPwmA = gpioPwmA, 
        .miGpioPwmB = gpioPwmB,
        .mfDuty = 0.f,
        .mxInitialized = false
    };

    return encDcMotor;
}

bool initializeMotor(DCMotor *pMotor) ;

void runMotorFwd(DCMotor *pMotor, float dutyCycle);
void runMotorBwd(DCMotor *pMotor, float dutyCycle);
void stopMotor(DCMotor *pMotor);
void runMotor(DCMotor *pMotor, float dutyCycle) ;

void printMotor(DCMotor *pMotor);


#endif // DCMOTOR_H