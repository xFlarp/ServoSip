#include "lcd.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include "ultra.h"
#include "pump.h"
#include "temp.h"
#include "keypad.h"

enum STATE { INTRO, INPUT_HEIGHT, SELECT, FILL, DONE, NEXT }; 
enum STATE currentState = INTRO;

extern float distance;
char buffer[16];

int cursor_pos = 0;
int dist_counter = 0;

char heightBuffer[6];
int heightIndex = 0;
float cupHeight = 0;



int main (void){
	LCD_port_init();
	LCD_init();
	pwm_init();
	incap_init();
	keypad_init();
	
	LCD_clearDisplay();
	LCD_printString("ServoSip");
	delay(2000);
	LCD_clearDisplay();

	currentState = INPUT_HEIGHT;
	LCD_placeCursor(1);
	LCD_printString("Set Height: ");
	LCD_placeCursorRC(2,0);
	
	
	while(1){
		char key = keypad_scan();
		/*distancecalc();
		
 if (dist_counter == 0){
        distancecalc();
        LCD_placeCursor(1);
        LCD_printString("Dist: ");
        LCD_printFloat(distance,2);
        LCD_printString(" in   ");
 } */
	 if (key != '\0') {
            if (currentState == INPUT_HEIGHT) {
                if (key >= '0' && key <= '9') {
                    if (heightIndex < 5) { // protect buffer
                        heightBuffer[heightIndex++] = key;
                        heightBuffer[heightIndex] = '\0'; // keep null-terminated
                        LCD_printChar(key);
                    }
                }
                else if (key == '#') {
                    // Done entering height
                    cupHeight = atof(heightBuffer); // convert to float
                    LCD_clearDisplay();
                    LCD_printString("Height set");
                    delay(1000);
                    LCD_clearDisplay();
                    currentState = NEXT;
                    
                    //
                    LCD_printFloat(cupHeight,2);
                }
            }
        }
        
        delay(50); // small delay for keypad stability
    }
}