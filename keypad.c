#include "keypad.h"
#include "gpio.h"

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
}

char keypad_scan(void){
	const char keys [4][3] = {
		{'1', '2', '3'},
		{'4', '5', '6'},
		{'7', '8', '9'},
		{'*', '0', '#'}
	};
	
	for (int row = 0; row<4; row++){
		//all rows high
		GPIOE->ODR |= (0xF <<12); //PE12-15 high
		//pull llow
		GPIOE->ODR &= ~(1<<(15-row)); //one by one
		
		for (int col = 0; col<3; col++){
			int pin = 11-col; //pe11, pe10, pe9
			if (!(GPIOE->IDR & (1<<pin))){
				while (!(GPIOE->IDR & (1<<pin))); //debounce
				return keys[row][col];
			}
		}
	}
	return '\0'; //no key pressed if no key pressed
}