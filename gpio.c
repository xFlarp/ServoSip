#include "gpio.h"

void GPIO_enable(unsigned long addr){
	RCC->AHB1ENR |= addr;
}

void GPIO_setOutput(GPIO_TypeDef* GPIOx, int32_t pin){
	GPIOx->MODER &= ~(3u<<pin*2);
	GPIOx->MODER |= (1u<<pin*2);
}

void GPIO_setInput(GPIO_TypeDef* GPIOx, int32_t pin){
	GPIOx->MODER &= ~(3u<<pin*2);
}

void GPIO_setPin(GPIO_TypeDef* port, uint8_t pin) {
    port->BSRR = (1u << pin);
}

void GPIO_clearPin(GPIO_TypeDef* port, uint8_t pin) {
    port->BSRR = (1u << (pin + 16));
}

/*******************************
 * GPIO_setAltFunction()
 * Configures a GPIO pin as an alternate function.
 * Inputs: 
 * - GPIOx: The GPIO port (e.g., GPIOA, GPIOB, etc.)
 * - pin: The pin number (0-15)
 * - af_value: The alternate function value (0 to 15)
 *******************************/
void GPIO_setAltFunction(GPIO_TypeDef* GPIOx, int32_t pin, uint8_t af_value) {
    GPIOx->MODER &= ~(3u << (pin * 2));  // Clear mode bits
    GPIOx->MODER |= (2u << (pin * 2));   // Set as alternate function mode (10)

    if (pin < 8) {
        GPIOx->AFR[0] &= ~(0xF << (pin * 4));  // Clear alternate function bits
        GPIOx->AFR[0] |= (af_value << (pin * 4)); // Set the AF value
    } else {
        GPIOx->AFR[1] &= ~(0xF << ((pin - 8) * 4));
        GPIOx->AFR[1] |= (af_value << ((pin - 8) * 4));
    }
}

void GPIO_writePin(GPIO_TypeDef* GPIOx, int32_t pin, int state) {
    if (state) {
        GPIOx->BSRR = (1u << pin);  // Set pin HIGH
    } else {
        GPIOx->BSRR = (1u << (pin + 16));  // Set pin LOW
    }
}

int GPIO_readPin(GPIO_TypeDef* GPIOx, int32_t pin) {
    return (GPIOx->IDR & (1u << pin)) ? 1 : 0;
}
/*******************************
 * GPIO_setPullResistor()
 * Configures the internal pull-up or pull-down resistor of a GPIO pin.
 * mode = 0 -> No pull-up/pull-down
 * mode = 1 -> Pull-up resistor enabled
 * mode = 2 -> Pull-down resistor enabled
 *******************************/
void GPIO_setPullResistor(GPIO_TypeDef* GPIOx, int32_t pin, uint8_t mode) {
    GPIOx->PUPDR &= ~(3u << (pin * 2));  // Clear current setting
    GPIOx->PUPDR |= (mode << (pin * 2)); // Set new resistor configuration
}