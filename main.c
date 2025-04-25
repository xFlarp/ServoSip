#include "lcd.h"
#include "gpio.h"
#include <stdio.h>
#include "ultra.h"
#include "pump.h"
#include "temp.h"
#include "keypad.h"

extern float distance;
char buffer[16];
int cursor_pos = 0;
int dist_counter = 0;

int main (void){
	LCD_port_init();
	LCD_init();
	//LCD_printString("ServoSip");
	LCD_placeCursor(2);
	//LCD_printString("Ben & Sam");
	pwm_init();
	incap_init();
	keypad_init();
	//delay(1000);
	
	
	while(1){
		distancecalc();
		
 if (dist_counter == 0){
        distancecalc();
        LCD_placeCursor(1);
        LCD_printString("Dist: ");
        LCD_printFloat(distance,2);
        LCD_printString(" in   ");
 }
		 char key = keypad_scan();
		 if (key != '\0'){
	   LCD_placeCursorRC(2, cursor_pos); 
	   LCD_printChar(key);
		 cursor_pos++;
	}
		   dist_counter++;
    if (dist_counter >= 10) { 
        dist_counter = 0;
    }
		
		delay(50);
		
		
	}
}