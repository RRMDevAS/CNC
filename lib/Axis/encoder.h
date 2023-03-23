#ifndef ENCODER_H
#define ENCODER_H

#include <driver/gpio.h>
#include <driver/pcnt.h>

struct Encoder {
    int64_t miEncoderTicks;
    int64_t miEncoderTicksMax;
    gpio_num_t miGpioA, miGpioB;
	pcnt_unit_t muUnit;
};
typedef struct Encoder Encoder;

static Encoder newEncoder(pcnt_unit_t unit, gpio_num_t pinA, gpio_num_t pinB) {
    Encoder result = { .miEncoderTicks= 0, 
                        .miEncoderTicksMax = 1000, 
                        .miGpioA = pinA, 
                        .miGpioB = pinB,
                        .muUnit = unit
        };

    return result;
}

static void pcntOverflowHandler(void *arg);

bool initializeEncoder(Encoder* pEncoder);

int64_t getRawCount(Encoder* pEncoder);
int64_t getCount(Encoder* pEncoder);

int64_t clearCount(Encoder* pEncoder);

int64_t pauseCount(Encoder* pEncoder);

int64_t resumeCount(Encoder* pEncoder);

void printEncoder(Encoder *pEncoder);

#endif  // ENCODER_H