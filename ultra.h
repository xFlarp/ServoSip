#include "stm32f4xx.h"

void pwm_init(void);
void incap_init(void);
void TIM3_IRQHandler(void);
float distancecalc(void);
void trigger_ultrasonic(void);