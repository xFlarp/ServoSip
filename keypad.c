#include "keypad.h"
#include "gpio.h"
#include "lcd.h"


volatile char lastKeyPressed = '\0';

static const char keys[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

void keypad_init(void){
	//GPIOE clock enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	
	//PE12-15 outputs (rows)
	GPIO_setOutput(GPIOE,12);
	GPIO_setOutput(GPIOE,13);
	GPIO_setOutput(GPIOE,14);
	GPIO_setOutput(GPIOE,15);
	//PE9-11 inputs (columns)
	GPIO_setInput(GPIOE,9);
	GPIO_setInput(GPIOE,10);
	GPIO_setInput(GPIOE,11);
	
	//Pull up for PE9-PE11
	GPIO_setPullResistor(GPIOE,11,1);
	GPIO_setPullResistor(GPIOE,10,1);
	GPIO_setPullResistor(GPIOE,9,1);
	
	//enable syscfg clock
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	//map exti9-11 to pe9-11
	SYSCFG->EXTICR[2] |= (0b0100 << 4*(9-8));
  SYSCFG->EXTICR[2] |= (0b0100 << 4*(10-8));
  SYSCFG->EXTICR[2] |= (0b0100 << 4*(11-8));
	
	//unmask interrupt
	EXTI->IMR |= (1 << 9) | (1 << 10) | (1 << 11);
	
	//rising edge trigger
	EXTI->RTSR |= (1 << 9) | (1 << 10) | (1 << 11);
	
	//enable interrupt
	NVIC_SetPriority(EXTI9_5_IRQn,0);
	NVIC_SetPriority(EXTI15_10_IRQn,0);
	NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

}


void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR & (1 << 9)) {
				EXTI->PR |= (1 << 9); // clear pending
				NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
				handleKeyPress();
    }
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & (1 << 10)) {
        EXTI->PR |= (1 << 10);
				NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
				handleKeyPress();
    }
    if (EXTI->PR & (1 << 11)) {
        EXTI->PR |= (1 << 11);
				NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
			  handleKeyPress();
    }
}

void handleKeyPress(void) {
	delay(20);
    char key = keypad_scan();
    if (key != '\0') {
        processKey(key);
    }
}

void processKey(char key) {
    lastKeyPressed = key;
}


char keypad_scan(void) {
    for (int row = 0; row < 4; row++) {
        //all rows high
        GPIOE->ODR |= (0xF << 12);

        //current row low
        GPIOE->ODR &= ~(1 << (15 - row));

        delay(2); //debounce

        for (int col = 0; col < 3; col++) {
            int pin = 11 - col; // PE11, PE10, PE9

            if (!(GPIOE->IDR & (1 << pin))) { //key press
                char key = keys[row][col];

                // Wait for release
                while (!(GPIOE->IDR & (1 << pin))) {
                    delay(1); //debounce
                }

                return key;
            }
        }
    }

    return '\0'; // no key pressed
}
