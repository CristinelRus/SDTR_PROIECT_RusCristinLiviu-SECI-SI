#include "ultrasonic.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#if NODE_SENSOR
static uint16_t elapsed(uint16_t start)
{
    return (uint16_t)((uint16_t)TIM2->CNT - start);
}
void ultrasonic_init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    TIM2->PSC = 71;
    TIM2->ARR = UINT16_MAX;
    TIM2->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 = TIM_CR1_CEN;
}
Reading ultrasonic_measure(uint8_t sequence)
{
    Reading invalid = {UINT16_MAX, RANGE_FAULT, sequence};
    if (GPIOA->IDR & SONAR_ECHO_PIN)
        return invalid;
    TIM2->CCER = 0;
    TIM2->CNT = 0;
    TIM2->SR = 0;
    TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC2P;
    GPIOA->BSRR = SONAR_TRIG_PIN;
    uint16_t start = (uint16_t)TIM2->CNT;
    while (elapsed(start) < 12u) {}
    GPIOA->BRR = SONAR_TRIG_PIN;
    const uint32_t captured = TIM_SR_CC1IF | TIM_SR_CC2IF;
    TickType_t deadline = xTaskGetTickCount();
    while ((TIM2->SR & captured) != captured) {
        if ((TickType_t)(xTaskGetTickCount() - deadline) >= pdMS_TO_TICKS(30)) {
            TIM2->CCER = 0;
            return invalid;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    TIM2->CCER = 0;
    if (TIM2->SR & (TIM_SR_CC1OF | TIM_SR_CC2OF))
        return invalid;
    uint16_t rise = TIM2->CCR1;
    uint16_t fall = TIM2->CCR2;
    /* Folosesc diferenta pe 16 biti, inclusiv cand contorul trece prin zero. */
    return reading_from_pulse((uint16_t)(fall - rise), sequence);
}
#else
void ultrasonic_init(void) {}
Reading ultrasonic_measure(uint8_t sequence)
{
    return (Reading){UINT16_MAX, RANGE_FAULT, sequence};
}
#endif
