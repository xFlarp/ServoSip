#include "lcd.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include "ultra.h"
#include "pump.h"
#include "temp.h"
#include "keypad.h"

enum STATE { INTRO, INPUT_HEIGHT, SELECT, FILL_AUTO, FILL_MANUAL, DONE, RESETTER, test, tester }; 
enum STATE currentState = INTRO;

extern uint8_t captureDone;
char buffer[16];

char heightBuffer[5];
int heightIndex = 0;
float cupHeight = 0;

extern volatile char lastKeyPressed;

int heightDisplayed = 0;
int selectDisplayed = 0;

int testStep = 0;



int main (void){
	LCD_port_init();
	LCD_init();
	pwm_init();
	incap_init();
	keypad_init();
	pump_init();
	
	LCD_clearDisplay();
	LCD_printString("ServoSip");
	delay(2000);
	//currentState = INPUT_HEIGHT;
	//cupHeight = 8;
	currentState = test;

	
	
while (1) {
	/**  disable interrupt for testing
	
        //char key = lastKeyPressed;
        //lastKeyPressed = '\0';
	
	**/
				char key = keypad_scan(); //temp polling

        switch (currentState) {
					
            case INPUT_HEIGHT:
						{
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
							}
								

            case SELECT:
						{
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
									}

						case FILL_AUTO:
{
    trigger_ultrasonic();
    delay(50);  // wait for echo

    float measured = distancecalc(); // update if captured
    float target = cupHeight - 0.5f;
    float filled = cupHeight - measured;
    if (filled < 0) filled = 0;

    LCD_clearDisplay();
    LCD_placeCursorRC(1, 0);
    sprintf(buffer, "Dist: %.2f", measured);
    LCD_printString(buffer);

    LCD_placeCursorRC(2, 0);
    sprintf(buffer, "Done:%d Fill:%.2f", captureDone, filled);
    LCD_printString(buffer);

    if (measured > target) {
        pump_run();
    } else {
        TIM4->CCR4 = 0;
    }

    break;
}

		
						case FILL_MANUAL:
{
    static char fillBuffer[5];
    static int fillIndex = 0;
    static float targetFill = 0;
    static uint8_t promptDisplayed = 0;

    if (!promptDisplayed) {
        LCD_clearDisplay();
        LCD_printString("Fill how many?");
        LCD_placeCursorRC(2, 0);
        fillIndex = 0;
        promptDisplayed = 1;
    }

    if (promptDisplayed == 1) {
        if (key >= '0' && key <= '9') {
            if (fillIndex == 0) {
                fillBuffer[fillIndex++] = key;
                fillBuffer[fillIndex] = '\0';
                LCD_printChar(key);

                fillBuffer[fillIndex++] = '.';
                fillBuffer[fillIndex] = '\0';
                LCD_printChar('.');
            }
            else if (fillIndex >= 2 && fillIndex < 4) {
                fillBuffer[fillIndex++] = key;
                fillBuffer[fillIndex] = '\0';
                LCD_printChar(key);
            }

            // Auto-submit when 3 digits are entered
            if (fillIndex == 4) {
                float requestedFill = atof(fillBuffer);
                if (requestedFill > cupHeight) {
                    LCD_clearDisplay();
                    LCD_printString("Too much!");
                    delay(1500);
                    promptDisplayed = 0;
                } else {
                    targetFill = requestedFill;
                    promptDisplayed = 2;
                    LCD_clearDisplay();
                    LCD_printString("Filling...");
                    delay(1000);
                }
            }
        }
        else if (key == '*' || key == '#') {
            LCD_clearDisplay();
            LCD_printString("Cancelled");
            delay(1000);
            currentState = SELECT;
            promptDisplayed = 0;
        }
    }

    // Filling loop
    if (promptDisplayed == 2) {
        if (key == '*' || key == '#') {
            pump_stop();
            LCD_clearDisplay();
            LCD_printString("Cancelled");
            delay(1000);
            currentState = DONE;
            promptDisplayed = 0;
            break;
        }

        trigger_ultrasonic();
        delay(50);
        float measured = distancecalc();
        float filled = cupHeight - measured;
        if (filled < 0) filled = 0;

        LCD_clearDisplay();
        LCD_placeCursorRC(1, 0);
        sprintf(buffer, "Filled: %.2f", filled);
        LCD_printString(buffer);

        LCD_placeCursorRC(2, 0);
        sprintf(buffer, "of %.2f in", targetFill);
        LCD_printString(buffer);

        if (filled < targetFill - 0.5f) {
            pump_run();
        } else {
            pump_stop();
            LCD_clearDisplay();
            LCD_printString("Done!");
            delay(2000);
            currentState = DONE;
            promptDisplayed = 0;
        }
    }

    break;
}

						case test:
{
    LCD_clearDisplay();
    LCD_printString("Dist: Reading...");
    delay(1000);

    for (int i = 0; i < 16; i++) { // ~5 seconds @ 300ms each
        float measured = distancecalc_avg(30);
        LCD_clearDisplay();
        LCD_placeCursorRC(1, 0);
        sprintf(buffer, "Dist: %.2f in", measured);
        LCD_printString(buffer);
        delay(300);
    }

    currentState = tester;
    break;
}




            case DONE:
						{
                break;
						}
						
						case RESETTER:
						{
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
						}
						
						case tester:
{
    LCD_clearDisplay();
    LCD_printString("Pumping...");
    pump_run();
    delay(1000);
    pump_stop();

    LCD_clearDisplay();
    LCD_printString("Switching back");
    delay(1000);

    currentState = test;
    break;
}

            default:
                break;
        }

    delay(5);
}
}