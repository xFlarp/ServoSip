/***************************************************************
Engineer: SAMUEL VILLALVA LIJO
TEMP SENSOR FILE for SERVOSIP



******************************************************************/

#include "temp.h"
#include <stdio.h>
#include <string.h>



//set up I2C to interface with temp sensor
//pb8 is scl, pb7 is sda.


//I2c MASTER set up

void I2C_Init(void){
	//init GPIOb
	//init pin 2 on AHB1
	RCC->AHB1ENR |= (1u<<1);
	//turn on GPIOC moder
	//run I2c INterface on pin pb6 and oin pb7
	GPIOB->MODER &=~(3u<<2*8);
	GPIOB->MODER |=(2u<<2*8);
	//set OTYPER to open drain
	GPIOB->OTYPER |=(1u<<8);
	//set OSPEEDER to high speed
	GPIOB->OSPEEDR &=~(3u<<2*8);
	GPIOB->OSPEEDR |=(2u<<2*8);
	//select pull up registers for pins
	GPIOB->PUPDR &=~(3u<<2*8);
	GPIOB->PUPDR |=(1u<<2*8);
	
	//set up alternate function for pin 6
	//afrl 24-27
	//i2c is af4 u8
	
	// Alternate Function 4 
	// PB7 in AFL pB8 in AFH
	GPIOB->AFR[1] |= 0x00000004; // PB8
	
	
	//set up pb7,
	GPIOB->MODER &=~(3u<<2*7);
	GPIOB->MODER |=(2u<<2*7);
	//set OTYPER to open drain
	GPIOB->OTYPER |=(1u<<7);
	//set OSPEEDER to high speed
	GPIOB->OSPEEDR &=~(3u<<2*7);
	GPIOB->OSPEEDR |=(2u<<2*7);
	//seelct pull up registers for pins
	GPIOB->PUPDR &=~(3u<<2*7);
	GPIOB->PUPDR |=(1u<<2*7);
	
	//set up alternate function for pin 7.
	//afrl 31-28
	//af4 i2c is 0x8
	GPIOB->AFR[0] |= 0x40000000; // PB7

//I2c set up
	//set up I2c1 periperal clock
	//bit 21 on APB1
	RCC->APB1ENR |=(1u<<21); //turn on I2c1 


    // Software reset I2C1
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;
		
		//enable ack
		I2C1->CR1 |= (1u<<10);

	//assume 16 mhz
	// Set peripheral clock freq in MHz (PCLK1 = 16 MHz)
   
	
	   // Set peripheral clock freq in MHz (PCLK1 = 16 MHz)
    I2C1->CR2 = 16;
	

	I2C1->CCR = 80;
	
	
	//TRISE = 17
	//trise = 2000ns/125ns = 16 +1
	I2C1->TRISE &=~(0x1fu<<0);
	I2C1->TRISE |=(0x11u<<0);
	
	//configure CR1 to enable periperal
	//set bit zero to 1.
	I2C1->CR1 |=(1u<<0);
	
}


 uint16_t Read_Temp(void){
	volatile uint32_t temp;
	uint8_t temp_raw_bin[2];  //create an array to hold raw binary temp data from sensor
	 I2C1->CR1 |= I2C_CR1_ACK; 
	
	//Genrate Start condition
	I2C1->CR1 |= I2C_CR1_START;
	while (!(I2C1->SR1 & I2C_SR1_SB)); // wait for start bit sent
	 //toggle scl to clear it and see if it works

	//send slave address and write to it
	I2C1->DR = 0x90;
	while (!(I2C1->SR1 & I2C_SR1_ADDR)); //wait for address ack
	temp = I2C1->SR2; //clear ADDR by reading SR1 and sr2
	
	//send register address, 0x00 on temp sensor
	while(!(I2C1->SR1 & I2C_SR1_TXE)); //wait unitl data register
	I2C1->DR = 0x00;
	
	while(!(I2C1->SR1 & I2C_SR1_BTF)); //wait for byte transfer finished
	
	//generate repeat start
	I2C1->CR1 |= I2C_CR1_START;
	while(!(I2C1->SR1 &I2C_SR1_SB)); // wait for start bit sent
	
	
	//send slave address read
	I2C1->DR = 0x91;
	while(!(I2C1->SR1 &I2C_SR1_ADDR)); //read sr1 and clear it
	temp = I2C1->SR2; //read and clear address
	
	//read 2 bytes

	
	while(!(I2C1->SR1 & I2C_SR1_RXNE)); //wait for first byte
	temp_raw_bin[0] = I2C1->DR;
	

	
	//read second byte
	while(!(I2C1->SR1 & I2C_SR1_RXNE));
	temp_raw_bin[1] = I2C1->DR;

		//generate STOP AFTER reading second byte
	I2C1->CR1 |= I2C_CR1_STOP;
	//combine bytes 
	int16_t result = (temp_raw_bin[0]<<8) | temp_raw_bin[1];
	
	result >>= 4;
	
	
	
	return result;

	
}
 
float temp_conversion(int temp_reading){
	//convert raw binary to int
	//perform conversion to temperature
	//return as float.
	int temp;
	int * input_temp = &temp_reading;
	memcpy(&temp, input_temp, sizeof(int));
	float temperture = 0.0625 *(float) temp;
	return temperture;
	
}

float TEMP(void){
	uint16_t temp;
	float final_temp;
	temp = Read_Temp();
	
	final_temp = temp_conversion(temp);
	return final_temp;
}