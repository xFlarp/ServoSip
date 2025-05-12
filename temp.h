#include "stm32f407xx.h"

void I2C_Init(void);

 uint16_t Read_Temp(void);
 
 float TEMP(void);
 
 float temp_conversion(int temp_reading);