#include "pump.h"
#include "gpio.h"
#include "lcd.h"

#define DIRA 13
#define DIRB 12


void pump_init(void) {
    // Enable clock for GPIOD (bit 3 in AHB1ENR)
    RCC->AHB1ENR |= (1u << 3);

    // Configure GPIOD Pin 15 as an Alternate Function (AF) for PWM output
    GPIOD->MODER &= ~(3u << (15 * 2)); // Clear mode bits for Pin 15
    GPIOD->MODER |= (1u << (15 * 2 + 1)); // Set mode to AF (10)
		//configure DIR pins as output
		GPIO_setOutput(GPIOD,DIRA);
		GPIO_setOutput(GPIOD,DIRB);
		//sets to push pull mode
		GPIOD->OTYPER &= ~(1u<<DIRA);
		GPIOD->OTYPER &= ~(1u<<DIRB);
	//set speed
			GPIOD->OSPEEDR |= (3u << (2 * DIRA));
			GPIOD->OSPEEDR |= (3u << (2 * DIRB));

    // Configure Pin 15 alternate function to TIM4 Channel 4 (AF2)
    GPIOD->AFR[1] &= ~(0xF << 28); // Clear AF selection bits for Pin 15
    GPIOD->AFR[1] |= (2u << 28); // Set AF2 for TIM4_CH4

    // Enable TIM4 clock in RCC APB1ENR register
    RCC->APB1ENR |= (1u << 2);

    // Configure TIM4 Channel 4 for PWM mode
    // Set CCMR2 output compare mode to PWM Mode 1 (OC4M = 110)
    TIM4->CCMR2 &= ~(7u << 12); // Clear OC4M bits
    TIM4->CCMR2 |= (6u << 12); // Set OC4M to PWM Mode 1 (110)

    // Set output compare channel 4 as output (CC4S = 00 in CCMR2)
    TIM4->CCMR2 &= ~(3u << 8);

    // Select polarity: Active high (CC4P = 0 in CCER)
    TIM4->CCER &= ~(1u << 13);

    // Set the auto-reload value (ARR) for the PWM frequency (max count value)
    TIM4->ARR = 999; // Defines PWM period (duty cycle resolution)

    // Set prescaler to achieve desired PWM frequency
    TIM4->PSC = 15; // Prescaler for 1 kHz PWM

    // Initialize duty cycle (CCR4 register)
    TIM4->CCR4 = TIM4->ARR;

    // Enable ARR buffering (Auto-reload preload) for smooth PWM updates
    TIM4->CR1 |= (1u << 7); // ARPE (Auto-reload preload enable)

    // Enable preload for Capture/Compare register (CCMR2 OC4PE)
    TIM4->CCMR2 |= (1u << 11); 

    // Set counting mode to up-counter (DIR = 0 in CR1)
    TIM4->CR1 &= ~(7u << 4);

    // Enable Capture/Compare 4 output (CCER CC4E)
    TIM4->CCER |= (1u << 12);

    // Enable TIM4 counter to start PWM generation
    TIM4->CR1 |= (1u << 0);
}

void pump_run(void){
		TIM4->CCR4 = TIM4->ARR;
		GPIOD->ODR |= (1u<<DIRA);
		GPIOD->ODR &= ~(1u<<DIRB);
}

void pump_stop(void){
		TIM4->CCR4 = 0;
		GPIOD->ODR &= ~(1u << DIRA);
    GPIOD->ODR &= ~(1u << DIRB);
}
