#include "axis.h"

#include "esp_log.h"

#include <math.h>

static const char *TAG = "AXIS";


AxisGpioPins newAxisGpioPins(gpio_num_t pwmA, gpio_num_t pwmB, gpio_num_t encA, gpio_num_t encB) {
    AxisGpioPins pins = {
        .gpioPwmA = pwmA,
        .gpioPwmB = pwmB,
        .gpioEncA = encA,
        .gpioEncB = encB
    };
    return pins;
}

void setAxisLead(Axis *pAxis, float lead) {
    pAxis->mfLead = lead;
    pAxis->mfMaxSpeed = 350.f * pAxis->mfLead;
    // full circle angle -> 2.f * 3.141592f
    // number of pulses per rotation -> 44
    // gearbox ratio -> 34.02
    // threaded rod lead - movement in mm per rotation -> 2.f
    // axis.mfEncoderRatio = 2.f * 3.141592f / 44.f / 34.02f * 2.f / 1.f;
    pAxis->mfEncoderRatio = 1.f / 44.f / 34.02f * pAxis->mfLead / 1.f;
}

bool initializeAxis(Axis *pAxis) {
    
    ESP_LOGI(TAG, "Init axis %i", pAxis->meAxis);
    
    bool xMotorInit = initializeMotor( &pAxis->mMotor );
    bool xEncoderInit = initializeEncoder( &pAxis->mEncoder );

    if (!xMotorInit || !xEncoderInit) {
        ESP_LOGE(TAG, "Failed to init axis %i", pAxis->meAxis);
        return false;
    }

    pAxis->mSpeedControl = newPidControl(20.0, 0.0, 0.0);

    pAxis->mxInitialized = true;

    return true;
}

void axisEncoderUpdate(Axis *pAxis, int64_t timeDelta) {
    pAxis->miTotalTime += timeDelta;

    float fTimeDelta = 0.0f;

    while (pAxis->miTotalTime-pAxis->miAccTime > 1000) {
        pAxis->miAccTime += 1000;

        int64_t iEncCount = getCount( &pAxis->mEncoder);
        clearCount(&pAxis->mEncoder);
        pAxis->miEncoderCount += iEncCount;
        float fCurrentPosition = pAxis->miEncoderCount * pAxis->mfEncoderRatio;
        pAxis->mfSpeed = (fCurrentPosition - pAxis->mfPosition) / 0.001f;
        // if (timeDelta>0.000001f) {
        //     pAxis->mfVelocity = (fCurrentPosition - pAxis->mfPosition) / timeDelta;
        // }
        pAxis->mfPosition = fCurrentPosition;

        fTimeDelta += 0.001f;
    }
    

    setPidCurrent(&(pAxis->mSpeedControl), pAxis->mfSpeed, fTimeDelta);
}

void axisMotorUpdate(Axis *pAxis) {
    float fPosDelta = pAxis->mfTargetPosition - pAxis->mfPosition;
    axisSetSpeed(pAxis, fPosDelta / 3.5f * 100.f);
    // ESP_LOGI(TAG, "position delta: %f", fPosDelta);
    // float fMotorDuty = fPosDelta / 3.5f * 100.f;      //  fMotorDuty = fPosDelta / 10.f [mm] * 100%
    float fMotorDuty = pAxis->mfTargetSpeed / pAxis->mfMaxSpeed * 100.f;      //  fMotorDuty = target speed / max speed * 100%

    // add pid control
    fMotorDuty += pAxis->mSpeedControl.mfControl / pAxis->mfMaxSpeed * 100.f;

    float fDutyLimit = 100.f;
    if (fMotorDuty>fDutyLimit) fMotorDuty = fDutyLimit;
    if (fMotorDuty<-fDutyLimit) fMotorDuty = -fDutyLimit;


    if (fMotorDuty>-3.f && fMotorDuty<3.f) {
        fMotorDuty = 0.f;
    } else if (fMotorDuty>-10.f && fMotorDuty<10.f) {
        if (fMotorDuty<0.f) {
            fMotorDuty = -10.f;
        } else {
            fMotorDuty = 10.f;
        }
    }

    int32_t iDuty = (int32_t)fMotorDuty;

    if (iDuty!=pAxis->miDuty) {
        pAxis->miDuty = iDuty;
        runMotor( &pAxis->mMotor, fMotorDuty * -1.f);
    }

}

void axisSetSpeed(Axis *pAxis, float speed) {
    // only set new target for pid controller if speed is different from one already set
    if (fabs(pAxis->mfTargetSpeed - speed) > 0.001f) {
        pAxis->mfTargetSpeed = speed;
        setPidTarget(&(pAxis->mSpeedControl), speed);
    }
}

void printAxis(Axis *pAxis) {
    ESP_LOGI(TAG, "------------------");
    ESP_LOGI(TAG, "position: %f", pAxis->mfPosition);
    ESP_LOGI(TAG, "velocity: %f", pAxis->mfSpeed);
    ESP_LOGI(TAG, "encoder: %lld", pAxis->miEncoderCount);
	// printf("%s: position: %f\n", TAG, pAxis->mfPosition);
	// printf("%s: velocity: %f\n", TAG, pAxis->mfVelocity);
    printEncoder( &pAxis->mEncoder );
    ESP_LOGI(TAG, "---");
}