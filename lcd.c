// lcd.c
#include "lcd.h"
#include <stdio.h>

void LCD_port_init(){
//STEP 1: Enable GPIOD in RCC AHB1ENR register
	RCC->AHB1ENR |= (1<<3);
   //clear ports
		GPIOD->MODER &= ~(3u<<2*7);
		GPIOD->MODER &= ~(3u<<2*6);
		GPIOD->MODER &= ~(3u<<2*5);
		GPIOD->MODER &= ~(3u<<2*3);
		GPIOD->MODER &= ~(3u<<2*2);
		GPIOD->MODER &= ~(3u<<2*1);
		GPIOD->MODER &= ~(3u<<2*0);
	//STEP 2: Set MODER of GPIOD Pins 7, 6, 5, 3, 2, 1 & 0 as outputs
		GPIOD->MODER |= (1u<<2*7);
		GPIOD->MODER |= (1u<<2*6);
		GPIOD->MODER |= (1u<<2*5);
		GPIOD->MODER |= (1u<<2*3);
		GPIOD->MODER |= (1u<<2*2);
		GPIOD->MODER |= (1u<<2*1);
		GPIOD->MODER |= (1u<<2*0);
//STEP 3: Set OTYPER of GPIOD Pins 7, 6, 5, 3, 2, 1 & 0 as push-pull
		GPIOD->OTYPER &= ~(1u << 7);
		GPIOD->OTYPER &= ~(1u << 6);
		GPIOD->OTYPER &= ~(1u << 5);
		GPIOD->OTYPER &= ~(1u << 3);
		GPIOD->OTYPER &= ~(1u << 2);
		GPIOD->OTYPER &= ~(1u << 1);
		GPIOD->OTYPER &= ~(1u << 0);

 
//Done with LCD port Initialization
}

/*******************************
 * LCD_init()
 * Inputs: NONE
 * Outputs: NONE
 * LCD Initialization
 * Read the manual carefully
 * We are doing INITIALIZATION BY INSTRUCTION
 * Don't rush it.
 *******************************
 */

void LCD_init(){

// STEP 1: Wait for 100ms for power-on-reset to take effect
// OK - nothing needs to be done here. 
// Keep the board powered. By the time your code is downloaded
// to flash and you are ready to start execution using the 
// debugger - 100ms will have passed

// STEP 2: Set RS pin LOW to send instructions
	clear_PIN(7);

// Send instructions using following format:
// Check Busy Falg; Set EN=HIGH; Send 4-bit instruction; Set EN=low;
	
	
// STEP 3a-3d: Set 4-bit mode (takes a total of 4 steps)
	set_PIN(5);
	GPIOD->ODR &= ~(15u << 0);
	GPIOD->ODR |= (3u << 0);
	clear_PIN(5);
	delay(2);

	set_PIN(5);
	GPIOD->ODR &= ~(15u << 0);
	GPIOD->ODR |= (3u<<0);
	clear_PIN(5);
	delay(2);
	
	set_PIN(5);
	GPIOD->ODR &= ~(15u << 0);
	GPIOD->ODR |= (3u<<0);
	clear_PIN(5);
	delay(10);
	
	set_PIN(5);
	GPIOD->ODR &= ~(15u << 0);
	GPIOD->ODR |= (2u<<0);
	clear_PIN(5);
	delay(2);

// STEP 4: Set 2 line display -- treats 16 char as 2 lines
//			001DL NF** (DL 0: 4bits; N= 1: 2 lines; F=0 : 5x8 display
LCD_sendInstr(0x28);

// STEP 5: Set DISPLAY to OFF
LCD_sendInstr(0x08);

// STEP 6: CLEAR DISPLAY
LCD_clearDisplay();

// STEP 7: SET ENTRY MODE - Auto increment; no scrolling
LCD_sendInstr(0x06);

// STEP 8: Set Display to ON with Cursor and Blink.
LCD_sendInstr(0x0F);	
}

/*******************************
 * LCD_placeCursor()
 * Inputs: unsigned integer linenumber
 * Outputs: NONE
 * sets Cursor position to
 * Line 1, character 1 (hex address 0x80)
 * or Line 2, character 1 (hex addres 0xC0)
 *
 *******************************
 */

void LCD_placeCursor(uint32_t lineno){
	if (lineno==1){
	LCD_sendInstr(0x80);
	}
	else if (lineno==2){
	LCD_sendInstr(0xC0);
	}
}

void delay(uint32_t ms){
	uint32_t i;
	for (i=0;i<(ms*1000);i++){
		}
	}

/*******************************
 * LCD_sendData()
 * Inputs: unsigned character data (8-bit)
 * Outputs: NONE
 * writes the character to LCD.
 * Since we are using 4-bit mode
 * this function will take the character (8-bit)
 * transmit upper 4 bits and then lower 4 bits.
 * make sure the RS, RW and EN signals are set to correct value
 * for each 4-bit. 
 * also make sure to check the BF
 *******************************
 */

void LCD_sendData(unsigned char data){
	check_BF();
	set_PIN(7);
	
//send upper bits
	set_PIN(5);
	GPIOD->ODR &= ~(15u<<0); //clear upper 4 bits (4-7)
	GPIOD->ODR |= ((data>>4) & 0x0F); //send upper 4 bits (data<<4) and put it in bits 0-3
	clear_PIN(5);
	
	//send lower bits
	set_PIN(5);
	GPIOD->ODR &= ~(15u<<0); //clear upper 4 bits 4-7
	GPIOD->ODR |= (data & 0x0F); //send lower 4 bits (data & 0x0F) and put it in bits 0-3
	clear_PIN(5);
	
	clear_PIN(7);
	
}

void LCD_printString(char str[]){
	uint32_t i=0;
	while(str[i] != '\0'){
		LCD_sendData(str[i]);
		i++;
			if (i==16){
			LCD_placeCursor(2);
			}
	}
	
}

void LCD_printChar(char c){
	LCD_sendData(c);
}

void LCD_printInt(int num){
	char buffer[16];
	sprintf(buffer,"%d",num);
	LCD_printString(buffer);
}

void LCD_printFloat(float num, int dec){
	char buffer[16];
	sprintf(buffer, "%.*f",dec,num);
	LCD_printString(buffer);
}
/*******************************
 * LCD_sendInstr()
 * Inputs: unsigned character INSTRUCTION (8-bit)
 * Outputs: NONE
 * Sends commands to LCD
 * We are using 4-bit mode but 
 * this function accepts (8-bit) character
 * as input. You can make the call on how to handle that.
 * make sure the RS, RW and EN signals are set to correct value
 * for each 4-bit part of instructions. 
 * also make sure to check the BF
 *******************************
 */

void LCD_sendInstr(unsigned char Instruction){
	
	clear_PIN(7);
	
	check_BF();
	clear_PIN(5);
	set_PIN(5);
	GPIOD->ODR &= ~(15u<<0);
	GPIOD->ODR |= ((Instruction/16)<<0);
	clear_PIN(5);
	
	check_BF();
	clear_PIN(5);
	set_PIN(5);
  GPIOD->ODR &= ~(15u<< 0);
	GPIOD->ODR |= ((Instruction % 16)<<0);
	clear_PIN(5);
}


/*******************************
 * LCD_clearDisplay()
 * Inputs: NONE
 * Outputs: NONE
 * Function to erase everything and 
 * clear LCD display
 *******************************
 */
void LCD_clearDisplay() {
	LCD_sendInstr(0x01);
}

/*******************************
 * clear_PIN()
 * Inputs: an integer PIN NUMBER (e.g. RW, EN)
 * Outputs: NONE
 * CLEARS PIN in GPIOD to 0
 * Read the Reference manual carefully
 * you can use the BSRR register without masks
 * OR you can use the ODR register WITH &~ (AND-NOT) mask 
 * to clear ONE specified pin.
 *******************************
 */
void clear_PIN(int PINNO){
	GPIOD->BSRR = (1<<(PINNO+16));
}

/*******************************
 * set_PIN()
 * Inputs: an integer PIN NUMBER (e.g. RW, EN)
 * Outputs: NONE
 * SETS PIN in GPIOD to 1
 * Read the Reference manual carefully
 * you can use the BSRR register without masks
 * OR you can use the ODR register WITH | (OR) mask 
 * to SET ONE specified pin.
 *******************************
 */
void set_PIN(int PINNO){
	GPIOD->BSRR = (1<<PINNO);
}

/*******************************
 * check_BF()
 * Inputs: NONE
 * Outputs: NONE
 * Checks BF flag on DB7 pin of LCD
 * and prevents code from moving ahead
 * if the BF flag is 1 (indicating LCD busy)
 *******************************
 */

void check_BF(){
	// STEP 1: Clear RS (set RS=0) as reading flag is an instruction
	clear_PIN(7);
		
	// STEP 2: set Data Pin 7 connected to GPIOD Pin 3 as input 
	// 		   (no pull-up or pull down setup needed here)
	GPIOD->MODER &= ~(3u<<2*3);
	
	// STEP 3: Set RW = 1 to read the BF flag.
	set_PIN(6);
	// STEP 4: Set EN = 1
	set_PIN(5);
	delay(5);

	// STEP 5: Read the BUSY FLAG on Pin 3 of GPIOD.
	//		   Wait here if BUSY and keep reading pin  
	//         until BF becomes 0 indicating NOT BUSY.
	while (GPIOD->IDR & (1<<3)){
		//loop until pin 3 becomes low (BF cleared)
	}

	// STEP 6: CLEAR EN =0
	clear_PIN(5);
	
	//STEP 7: CLEAR RW =0 
	clear_PIN(6);

	//STEP 8: Set Data Pin 7 connected to GPIOD Pin 3 as output
	GPIOD->MODER &= ~(3u<<2*3);
	GPIOD->MODER |= (1u<<2*3);
}