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

char heightBuffer[5];
int heightIndex = 0;
float cupHeight = 0;

extern volatile char lastKeyPressed;



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
	
	
while (1) {
    if (lastKeyPressed != '\0') {
        char key = lastKeyPressed;
        lastKeyPressed = '\0';

        switch (currentState) {
            case INPUT_HEIGHT:
                if (key >= '0' && key <= '9') {
                    if (heightIndex == 0) {
                        heightBuffer[heightIndex++] = key;
                        heightBuffer[heightIndex] = '\0';
                        LCD_printChar(key);

                        heightBuffer[heightIndex++] = '.';
                        heightBuffer[heightIndex] = '\0';
                        LCD_printChar('.');
                    }
                    else if (heightIndex >= 2 && heightIndex < 4) {
                        heightBuffer[heightIndex++] = key;
                        heightBuffer[heightIndex] = '\0';
                        LCD_printChar(key);
                    }
                }
                else if (key == '#') {
                    cupHeight = atof(heightBuffer);

                    if (cupHeight > 9.99f) {
                        cupHeight = 9.99f;
                    }

                    LCD_clearDisplay();
                    LCD_printString("Height set");
                    LCD_placeCursor(2);
                    LCD_printFloat(cupHeight,2);
                    LCD_printString(" in");
                    delay(2000);
                    LCD_clearDisplay();
                    
                    currentState = SELECT;
                    LCD_printString("Select Mode:");
                    LCD_placeCursorRC(2, 0);
                    LCD_printString("1.Auto 2.Manual");
                }
                break;

            case SELECT:
                
                break;

            case FILL:
                break;

            case DONE:
                break;

            default:
                break;
        }
    }

    delay(5);
}
}