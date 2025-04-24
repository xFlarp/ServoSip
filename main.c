#include "lcd.h"
#include "gpio.h"
#include <stdio.h>
#include "ultra.h"
#include "pump.h"
#include "temp.h"

extern float distance;
char buffer[16];

int main (void){
	LCD_port_init();
	LCD_init();
	LCD_printString("ServoSip");
	LCD_placeCursor(2);
	LCD_printString("Ben & Sam");
	pwm_init();
	incap_init();
	delay(1000);
	
	
	while(1){
		distancecalc();
		LCD_clearDisplay();
		LCD_printString("Dist: ");
		LCD_printFloat(distance,2);
		LCD_printString(" in");
		delay(2000);
		
	}
}