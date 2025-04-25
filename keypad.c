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