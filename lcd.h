// lcd.h
#ifndef LCD_H
#define LCD_H

#include "stm32f4xx.h"

/*******************************
 * LCD pins connections to PORTD
 *******************************
 */

#define RS 7 
#define RW 6 
#define EN 5 
#define DB7 3 
#define DB6 2 
#define DB5 1 
#define DB4 0 



// Function prototypes
void LCD_port_init(void);
void LCD_init(void);
void LCD_placeCursor(uint32_t lineno);
void LCD_placeCursorRC(uint8_t row, uint8_t col);
void LCD_sendData(unsigned char data);
void LCD_sendInstr(unsigned char Instruction);
void LCD_clearDisplay(void);

// Additional functions for various data types
void LCD_printChar(char c);
void LCD_printString(char text[]);
void LCD_printInt(int number);
void LCD_printFloat(float number, int decimal_places);

// Helper Functions
void clear_PIN(int PINNO);
void set_PIN(int PINNO);
void check_BF(void);


// Feel free to add any other helpful functions
void delay(uint32_t);


#endif
