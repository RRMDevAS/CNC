#include "axis.h"

#include "esp_log.h"

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

void initializeAxis(Axis *pAxis) {
    initializeMotor( &pAxis->mMotor );
    initializeEncoder( &pAxis->mEncoder );

    pAxis->mxInitialized = true;
}

void axisEncoderUpdate(Axis *pAxis, int64_t timeDelta) {
    pAxis->miTotalTime += timeDelta;
    while (pAxis->miTotalTime-pAxis->miAccTime > 1000) {
        pAxis->miAccTime += 1000;

        int64_t iEncCount = getCount( &pAxis->mEncoder);
        clearCount(&pAxis->mEncoder);
        pAxis->miEncoderCount += iEncCount;
        float fCurrentPosition = pAxis->miEncoderCount * pAxis->mfEncoderRatio;
        pAxis->mfVelocity = (fCurrentPosition - pAxis->mfPosition) / 0.001f;
        // if (timeDelta>0.000001f) {
        //     pAxis->mfVelocity = (fCurrentPosition - pAxis->mfPosition) / timeDelta;
        // }
        pAxis->mfPosition = fCurrentPosition;
    }
    
}

void axisMotorUpdate(Axis *pAxis) {
    float fPosDelta = pAxis->mfTargetPosition - pAxis->mfPosition;
    // ESP_LOGI(TAG, "position delta: %f", fPosDelta);
    float fMotorDuty = fPosDelta / 3.5f * 100.f;      //  fMotorDuty = fPosDelta / 10.f [mm] * 100%
    float fDutyLimit = 100.f;
    if (fMotorDuty>fDutyLimit) fMotorDuty = fDutyLimit;
    if (fMotorDuty<-fDutyLimit) fMotorDuty = -fDutyLimit;

    if (fMotorDuty>-3.f && fMotorDuty<3.f) {
        fMotorDuty = 0.f;
    } else if (fMotorDuty>-6.f && fMotorDuty<6.f) {
        if (fMotorDuty<0.f) {
            fMotorDuty = -6.f;
        } else {
            fMotorDuty = 6.f;
        }
    }

    int32_t iDuty = (int32_t)fMotorDuty;

    if (iDuty!=pAxis->miDuty) {
        pAxis->miDuty = iDuty;
        runMotor( &pAxis->mMotor, fMotorDuty * -1.f);
    }

}

void printAxis(Axis *pAxis) {
    ESP_LOGI(TAG, "------------------");
    ESP_LOGI(TAG, "position: %f", pAxis->mfPosition);
    ESP_LOGI(TAG, "velocity: %f", pAxis->mfVelocity);
    ESP_LOGI(TAG, "encoder: %lld", pAxis->miEncoderCount);
	// printf("%s: position: %f\n", TAG, pAxis->mfPosition);
	// printf("%s: velocity: %f\n", TAG, pAxis->mfVelocity);
    printEncoder( &pAxis->mEncoder );
    ESP_LOGI(TAG, "---");
}