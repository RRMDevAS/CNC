#ifndef AXIS_H
#define AXIS_H

#include "encoder.h"
#include "dcmotor.h"

typedef enum {
    eX = 0,
    eY,
    eZ,
    eAxisCount
} EAxis;

struct AxisGpioPins{
    gpio_num_t gpioPwmA;
    gpio_num_t gpioPwmB; 
    gpio_num_t gpioEncA;
    gpio_num_t gpioEncB;
};
typedef struct AxisGpioPins AxisGpioPins;

AxisGpioPins newAxisGpioPins(gpio_num_t pwmA, gpio_num_t pwmB, gpio_num_t encA, gpio_num_t encB);

struct Axis {
    EAxis meAxis;

    Axle mMotorAxle;

    DCMotor mMotor;
    Encoder mEncoder;

    float mfPosition;
    float mfVelocity;
    float mfEncoderRatio;
    float mfPositionMin, mfPositionMax;
    float mfTargetPosition;

    int32_t miDuty;

    int64_t miAccTime, miTotalTime;
    int64_t miEncoderCount;

    bool mxInitialized;
};
typedef struct Axis Axis;

static Axis newAxis(EAxis axisId, 
                    gpio_num_t gpioPwmA, gpio_num_t gpioPwmB, 
                    gpio_num_t gpioEncA, gpio_num_t gpioEncB) {
    Axis axis;

    axis.meAxis = axisId;

    axis.mMotor = newMotor(0, axis.meAxis, gpioPwmA, gpioPwmB);
    axis.mEncoder = newEncoder(axis.meAxis, gpioEncA, gpioEncB);

    axis.mMotorAxle.mfAngle = 0.f;
    axis.mMotorAxle.mfOmega = 0.f;

    axis.mfPosition = 0.f;
    axis.mfVelocity = 0.f;
    // full circle angle -> 2.f * 3.141592f
    // number of pulses per rotation -> 44
    // gearbox ratio -> 34.02
    // threaded rod movement in mm per rotation -> 2.f
    // axis.mfEncoderRatio = 2.f * 3.141592f / 44.f / 34.02f * 2.f / 1.f;
    axis.mfEncoderRatio = 1.f / 44.f / 34.02f * 2.f / 1.f;

    axis.miDuty = 0;

    axis.miAccTime = 0;
    axis.miTotalTime = 0;

    axis.miEncoderCount = 0;

    axis.mxInitialized = false;

    return axis;
}

void initializeAxis(Axis *pAxis);

void axisEncoderUpdate(Axis *pAxis, int64_t timeDelta);
void axisMotorUpdate(Axis *pAxis);

void printAxis(Axis *pAxis);

#endif      // AXIS_H