#include "stm32f4xx.h"

void GPIO_enable(unsigned long addr);
void GPIO_setOutput(GPIO_TypeDef* GPIOx, int32_t pin);
void GPIO_setInput(GPIO_TypeDef* GPIOx, int32_t pin);
void GPIO_setAltFunction(GPIO_TypeDef* GPIOx, int32_t pin, uint8_t af_value);
void GPIO_writePin(GPIO_TypeDef* GPIOx, int32_t pin, int state);
int GPIO_readPin(GPIO_TypeDef* GPIOx, int32_t pin);
void GPIO_setPullResistor(GPIO_TypeDef* GPIOx, int32_t pin, uint8_t mode);
void GPIO_setPin(GPIO_TypeDef* port, uint8_t pin);
void GPIO_clearPin(GPIO_TypeDef* port, uint8_t pin);