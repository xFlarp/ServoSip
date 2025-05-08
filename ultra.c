#include "ultra.h"
#include "gpio.h"
#include "lcd.h"

volatile uint32_t risingtime = 0;
volatile uint32_t fallingtime = 0;
volatile uint8_t captureDone = 0;
volatile float distance = 0;

volatile uint8_t expectingRisingEdge = 1;


void pwm_init(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; //enable clock
	
	GPIOB->MODER &= ~(3u<<4*2); //set PB4 to alternate function
	GPIOB->MODER |= (1u<<9);
	
	GPIOB->AFR[0] &= ~(0xF<<16); //set alternate function mode AF2 for TIM3
	GPIOB->AFR[0] |= (1u<<17);
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; //clock for TIM3
	
	TIM3->ARR = 750000; //preset values
	TIM3->PSC = 15;
	TIM3->CCR1 = 10;
	
	TIM3->CCMR1 |= (6u<<4); //PWM mode 1 tim3
	
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE; //enable preload register
	
	TIM3->CCER |= TIM_CCER_CC1E; //enable capture compare for TIM3
	
	TIM3->CR1 |= TIM_CR1_ARPE; //enable auto-reload preload
	TIM3->CR1 |= TIM_CR1_CEN; //enable counter
}

void incap_init (void){
	GPIOB->MODER &= ~(3u<<5*2); //alternate function mode PB5
	GPIOB->MODER |= (1u<<11);
	
	GPIOB->AFR[0] &= ~(0xF<<20); //set AF2 for PB5
	GPIOB->AFR[0] |= (1u<<21);
	
	TIM3->CCMR1 |= (1u<<8); //set input capture mapped to TI2
	
	TIM3->CCER |= (TIM_CCER_CC2E); //enable capture for channel 2
	
	TIM3->CCER |= (TIM_CCER_CC2P); //capture on falling edge
	
	TIM3->DIER |= TIM_DIER_CC2IE; //enable interrupt on capture compare channel 2
	
	NVIC_EnableIRQ(TIM3_IRQn); //enable interrupt
}

void TIM3_IRQHandler(void){
    if (TIM3->SR & TIM_SR_CC2IF){
        if (expectingRisingEdge) {
            risingtime = TIM3->CCR2;
            TIM3->CCER |= TIM_CCER_CC2P; // Next: falling
            expectingRisingEdge = 0;
        } else {
            fallingtime = TIM3->CCR2;
            TIM3->CCER &= ~TIM_CCER_CC2P; // Next: rising
            captureDone = 1;
            expectingRisingEdge = 1;
        }
        TIM3->SR &= ~TIM_SR_CC2IF;
    }
}


void trigger_ultrasonic(void) {
    GPIO_setPin(GPIOB, 4);  // PB4 high
    delay(1);               // ~10–20 µs pulse
    GPIO_clearPin(GPIOB, 4); // PB4 low
}

float distancecalc(void) {
    if (captureDone) {
        captureDone = 0;

        uint32_t width;
        if (fallingtime >= risingtime) {
            width = fallingtime - risingtime;
        } else {
            width = (TIM3->ARR - risingtime + fallingtime + 1);
        }

        distance = (width / 148.0f);
    }

    return distance; // always return the last known value
}

float distancecalc_avg(int samples) {
    float sum = 0.0f;
    int count = 0;

    for (int i = 0; i < samples; i++) {
        trigger_ultrasonic();
        delay(5);

        float reading = distancecalc();
        if (reading > 0.0f && reading < 400.0f) {  // ignore bad readings
            sum += reading;
            count++;
        }
    }

    if (count == 0) {
        return -1.0f;  // error
    }

    return sum / count;
}






