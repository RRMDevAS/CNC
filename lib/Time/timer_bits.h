#ifndef TIMER_BITS_H
#define TIMER_BITS_H

#include <inttypes.h>
#include <stdbool.h>

struct TimerBits {
    int64_t miAccum;
    int64_t miSecond, miTenth, miFifth, miHalf;

    bool mxImpuls_0_1s, mxImpuls_0_2s, mxImpuls_0_5s, mxImpuls_1s, mxImpuls_2s, mxImpuls_3s, mxImpuls_5s;
    bool mxTact_0_5s, mxTact_1s, mxTact_2s, mxTact_0_2s;
};
typedef struct TimerBits TimerBits;

TimerBits newTimerBits();

void updateTimerBits(TimerBits* timerBits, int64_t delta);

#endif      // TIMER_BITS_H