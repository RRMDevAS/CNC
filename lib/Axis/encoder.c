#include "encoder.h"

#include <driver/gpio.h>
#include <driver/pcnt.h>
#include <hal/pcnt_hal.h>
#include <esp_log.h>

static const char *TAG = "ENCODER";

#define ENC_PCNT_MAX 200
#define ENC_PCNT_MIN -200

static void pcntOverflowHandler(void *arg)
{
    Encoder *enc = (Encoder *)arg;
    uint32_t status = 0;
    pcnt_get_event_status(enc->muUnit, &status);

    if (status & PCNT_EVT_H_LIM) {
        enc->miEncoderTicks += ENC_PCNT_MAX;
    } else if (status & PCNT_EVT_L_LIM) {
        enc->miEncoderTicks += ENC_PCNT_MIN;
    }
}

bool initializeEncoder(Encoder* pEncoder) {
	// Set up encoder PCNT configuration
	// channel 0
    pcnt_config_t encConfig = {
        .pulse_gpio_num = pEncoder->miGpioA,
        .ctrl_gpio_num = pEncoder->miGpioB,
        .channel = PCNT_CHANNEL_0,
        .unit = pEncoder->muUnit,
        .pos_mode = PCNT_COUNT_DEC,
        .neg_mode = PCNT_COUNT_INC,
        .lctrl_mode = PCNT_MODE_REVERSE,
        .hctrl_mode = PCNT_MODE_KEEP,
        .counter_h_lim = ENC_PCNT_MAX,
        .counter_l_lim = ENC_PCNT_MIN,
    };
	esp_err_t configRes = pcnt_unit_config(&encConfig);
	if (configRes!=ESP_OK) {
		ESP_LOGE(TAG, "Config failed");
		return false;
	}

	// channel 1
    encConfig.pulse_gpio_num = pEncoder->miGpioB;
    encConfig.ctrl_gpio_num = pEncoder->miGpioA;
    encConfig.channel = PCNT_CHANNEL_1;
    encConfig.pos_mode = PCNT_COUNT_INC;
    encConfig.neg_mode = PCNT_COUNT_DEC;

	pcnt_unit_config(&encConfig);


	// Filter out bounces and noise
	pcnt_set_filter_value(pEncoder->muUnit, 25000);  // Filter Runt Pulses
	pcnt_filter_enable(pEncoder->muUnit);


	/* Enable events on maximum and minimum limit values */
	pcnt_event_enable(pEncoder->muUnit, PCNT_EVT_H_LIM);
	pcnt_event_enable(pEncoder->muUnit, PCNT_EVT_L_LIM);

	pcnt_counter_pause(pEncoder->muUnit); // Initial PCNT init
	pcnt_counter_clear(pEncoder->muUnit);


    // register interrupt handler
	esp_err_t err = pcnt_isr_service_install(0);
	if (err==ESP_OK) {
    	pcnt_isr_handler_add(pEncoder->muUnit, pcntOverflowHandler, pEncoder);
	} else {
    	ESP_LOGE(TAG, "Failed to install isr service");
	}
	pcnt_counter_resume(pEncoder->muUnit);

	return true;
}

int64_t getRawCount(Encoder* pEncoder) {
	int16_t c=0;
	pcnt_get_counter_value(pEncoder->muUnit, &c);
	return c;
}
int64_t getCount(Encoder* pEncoder) {
	return getRawCount(pEncoder) + pEncoder->miEncoderTicks;
}

int64_t clearCount(Encoder* pEncoder) {
    pEncoder->miEncoderTicks = 0;
	return pcnt_counter_clear(pEncoder->muUnit);
}

int64_t pauseCount(Encoder* pEncoder) {
	return pcnt_counter_pause(pEncoder->muUnit);
}

int64_t resumeCount(Encoder* pEncoder) {
	return pcnt_counter_resume(pEncoder->muUnit);
}

void printEncoder(Encoder *pEncoder) {
	char mastrEnc[20];
	sprintf(mastrEnc, "%i", (int32_t)getCount(pEncoder));
    ESP_LOGI(TAG, "%s", mastrEnc);
	// printf("%s: %lld\n", TAG, getCount(pEncoder));
}