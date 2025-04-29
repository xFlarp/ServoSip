#include "lcd.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include "ultra.h"
#include "pump.h"
#include "temp.h"
#include "keypad.h"

enum STATE { INTRO, INPUT_HEIGHT, SELECT, FILL_AUTO, FILL_MANUAL, DONE, RESETTER }; 
enum STATE currentState = INTRO;

extern float distance;
char buffer[16];

char heightBuffer[5];
int heightIndex = 0;
float cupHeight = 0;

extern volatile char lastKeyPressed;

int heightDisplayed = 0;
int selectDisplayed = 0;


int main (void){
	LCD_port_init();
	LCD_init();
	pwm_init();
	incap_init();
	keypad_init();
	
	LCD_clearDisplay();
	LCD_printString("ServoSip");
	delay(2000);
	currentState = INPUT_HEIGHT;

	
	
while (1) {
	/**  disable interrupt for testing
	
        //char key = lastKeyPressed;
        //lastKeyPressed = '\0';
	
	**/
				char key = keypad_scan(); //temp polling

        switch (currentState) {
					
            case INPUT_HEIGHT:
							if (!heightDisplayed){
								LCD_clearDisplay();
								LCD_placeCursor(1);
								LCD_printString("Set Height: ");
								LCD_placeCursor(2);
								heightDisplayed = 1;
							}
								
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
                    currentState = SELECT;
                }
								break;
								

            case SELECT:
							if (!selectDisplayed){
										LCD_clearDisplay();
                    LCD_printString("Select Mode:");
                    LCD_placeCursorRC(2, 0);
                    LCD_printString("1.Auto 2.Manual");
										selectDisplayed=1;
							}
										if (key == '1'){
											LCD_clearDisplay();
											LCD_printString("Begin Auto");
											delay(1000);
											currentState = FILL_AUTO;
											selectDisplayed = 0;
										}
										else if (key == '2'){
											LCD_clearDisplay();
											LCD_printString("Begin Manual");
											delay(1000);
											currentState =  FILL_MANUAL;
											selectDisplayed = 0;
										}
										else if (key == '*'){
											currentState = RESETTER;
										}
										else if (key == '#'){
											currentState = RESETTER;
										}
										else if (key != '\0'){
											LCD_clearDisplay();
											LCD_printString("Invalid");
											delay(1000);
											selectDisplayed=0;
										}
                break;

            case FILL_AUTO:
                break;
						
						case FILL_MANUAL:
							break;

            case DONE:
                break;
						
						case RESETTER:
							LCD_clearDisplay();
						  LCD_printString("Resetting");
							delay(500);
							LCD_clearDisplay();
							cupHeight = 0;
							heightIndex = 0;
							selectDisplayed = 0;
							heightDisplayed = 0;
							currentState = INPUT_HEIGHT;
							break;

            default:
                break;
        }

    delay(5);
}
}