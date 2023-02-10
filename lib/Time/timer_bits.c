#include "timer_bits.h"

TimerBits newTimerBits() {
    TimerBits res = {
        .miAccum = 0,
        .miSecond = 0,
        .miTenth = 0,
        .miFifth = 0,
        .miHalf = 0,

        .mxImpuls_1s = false,
        .mxImpuls_0_1s = false,
        .mxImpuls_0_2s = false,
        .mxImpuls_0_5s = false,
        .mxImpuls_2s = false,
        .mxImpuls_3s = false,
        .mxImpuls_5s = false,
        .mxTact_0_5s = false, 
        .mxTact_1s = false, 
        .mxTact_2s = false,
        .mxTact_0_2s = false
    };

    return res;
}

void updateTimerBits(TimerBits* timerBits, int64_t delta) {
    timerBits->miAccum += delta;

    // second
    int64_t iTemp = timerBits->miAccum / (int64_t)1000000;
    timerBits->mxImpuls_1s = timerBits->miSecond != iTemp;
    timerBits->miSecond = iTemp;

    // tenth of a second
    iTemp = timerBits->miAccum / (int64_t)100000;
    timerBits->mxImpuls_0_1s = timerBits->miTenth != iTemp;
    timerBits->miTenth = iTemp;

    // fifth of a second
    iTemp = timerBits->miAccum / (int64_t)200000;
    timerBits->mxImpuls_0_2s = timerBits->miFifth != iTemp;
    timerBits->miFifth = iTemp;

    // half of a second
    iTemp = timerBits->miAccum / (int64_t)500000;
    timerBits->mxImpuls_0_5s = timerBits->miHalf != iTemp;
    timerBits->miHalf = iTemp;
    
    // 2s
    timerBits->mxImpuls_2s = (timerBits->miSecond % (int64_t)2) == 0 && timerBits->mxImpuls_1s;
    // 3s
    timerBits->mxImpuls_3s = (timerBits->miSecond % (int64_t)3) == 0 && timerBits->mxImpuls_1s;
    // 5s
    timerBits->mxImpuls_5s = (timerBits->miSecond % (int64_t)5) == 0 && timerBits->mxImpuls_1s;

    // tact 0.5s
    timerBits->mxTact_0_5s = (timerBits->miAccum % (int64_t)1000000) < 500000;
    // tact 1s
    timerBits->mxTact_1s = (timerBits->miAccum % (int64_t)2000000) < 1000000;
    // tact 2s
    timerBits->mxTact_2s = (timerBits->miAccum % (int64_t)4000000) < 2000000;
    // tact 0.2s
    timerBits->mxTact_0_2s = (timerBits->miAccum % (int64_t)400000) < 200000;
}