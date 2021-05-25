#include "dcmotor.h"

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"


static const char *TAG = "AXIS";

void initializeMotor(DCMotor *pMotor) {
    printf("initializeMotor - initializing mcpwm gpio...\n");
    esp_err_t pwmResA = mcpwm_gpio_init(pMotor->miMCPUnit, pMotor->mePwmSignalA, pMotor->miGpioPwmA);
    if (pwmResA!=ESP_OK) {
        ESP_LOGW(TAG, "Failed to init motor A pwm");
    }
    esp_err_t pwmResB = mcpwm_gpio_init(pMotor->miMCPUnit, pMotor->mePwmSignalB, pMotor->miGpioPwmB);
    if (pwmResB!=ESP_OK) {
        ESP_LOGW(TAG, "Failed to init motor B pwm");
    }

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 500;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    esp_err_t pwmResInit = mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    if (pwmResInit!=ESP_OK) {
        ESP_LOGW(TAG, "Failed to init mcpwm config");
    }

    pMotor->mxInitialized = true;
}

void runMotorFwd(DCMotor *pMotor, float dutyCycle) {
    if (!pMotor->mxInitialized) {
        return;
    }
    // ESP_LOGI("MOTOR", "BWD");
    mcpwm_set_signal_low(pMotor->miMCPUnit, pMotor->meTimer, MCPWM_OPR_A);
    mcpwm_set_duty(pMotor->miMCPUnit, pMotor->meTimer, MCPWM_OPR_B, dutyCycle * (1.f));
    mcpwm_set_duty_type(pMotor->miMCPUnit, pMotor->meTimer, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}
void runMotorBwd(DCMotor *pMotor, float dutyCycle) {
    if (!pMotor->mxInitialized) {
        return;
    }
            // ESP_LOGI("MOTOR", "FWD");
    mcpwm_set_signal_low(pMotor->miMCPUnit, pMotor->meTimer, MCPWM_OPR_B);
    mcpwm_set_duty(pMotor->miMCPUnit, pMotor->meTimer, MCPWM_OPR_A, dutyCycle);
    mcpwm_set_duty_type(pMotor->miMCPUnit, pMotor->meTimer, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}
void stopMotor(DCMotor *pMotor) {
    if (!pMotor->mxInitialized) {
        return;
    }
    // ESP_LOGI("MOTOR", "STOP");
    mcpwm_set_signal_low(pMotor->miMCPUnit, pMotor->meTimer, MCPWM_OPR_A);
    mcpwm_set_signal_low(pMotor->miMCPUnit, pMotor->meTimer, MCPWM_OPR_B);
}

void runMotor(DCMotor *pMotor, float dutyCycle) {
    if (!pMotor->mxInitialized) {
        return;
    }
    // ESP_LOGI("MOTOR", "duty: %f", dutyCycle);
    if (pMotor->mfDuty!=dutyCycle) {
        // ESP_LOGI("MOTOR", "old duty: %f", pMotor->mfDuty);
        if (dutyCycle<0.f) {
            runMotorBwd(pMotor, -dutyCycle);
        } else if (dutyCycle>0.f) {
            runMotorFwd(pMotor, dutyCycle);
        } else {
            stopMotor(pMotor);
        }
        // vTaskDelay(1000 / portTICK_RATE_MS);
        pMotor->mfDuty = dutyCycle;
    }
}

void printMotor(DCMotor *pMotor){

}
