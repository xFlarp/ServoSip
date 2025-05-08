#include "lcd.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include "ultra.h"
#include "pump.h"
#include "temp.h"
#include "keypad.h"

void timer2_init(void);

enum STATE { INPUT_HEIGHT, SELECT, FILL_AUTO, FILL_MANUAL, DONE, RESETTER }; 
enum STATE currentState = INPUT_HEIGHT;

extern uint8_t captureDone;
char buffer[16];

char heightBuffer[4];
int heightIndex = 0;
float cupHeight = 0;

extern volatile char lastKeyPressed;

int heightDisplayed = 0;
int selectDisplayed = 0;




int main (void){
	LCD_port_init();
	timer2_init();
	LCD_init();
	pwm_init();
	incap_init();
	keypad_init();
	pump_init();
	lastKeyPressed = '\0';
	
	LCD_clearDisplay();
	LCD_printString("ServoSip");
	delay(2000);
	currentState = INPUT_HEIGHT;

	
	
while (1) {
	
        char key = lastKeyPressed;
	

        switch (currentState) {
					
           case INPUT_HEIGHT:
{
    if (!heightDisplayed) {
        LCD_clearDisplay();
        LCD_placeCursor(1);
        LCD_printString("Set Height:");
        LCD_placeCursor(2);
        heightIndex = 0;
        heightDisplayed = 1;
    }

    if (key >= '0' && key <= '9') {
        if (heightIndex == 0) {
            heightBuffer[heightIndex++] = key;
            heightBuffer[heightIndex++] = '.';
            LCD_printChar(key);
            LCD_printChar('.');
						lastKeyPressed = '\0';
        }
        else if (heightIndex == 2) {
            heightBuffer[heightIndex++] = key;
            LCD_printChar(key);
						lastKeyPressed = '\0';

            // Null terminate
            heightBuffer[heightIndex] = '\0';

            // Convert and move on
            cupHeight = atof(heightBuffer);
            if (cupHeight > 9.9f) cupHeight = 9.9f;

            LCD_clearDisplay();
            LCD_printString("Height Set:");
            LCD_placeCursorRC(2, 0);
            LCD_printFloat(cupHeight, 1);
            LCD_printString(" in");
            delay(1500);

            currentState = SELECT;
            heightDisplayed = 0;
        }
    }
    else if (key == '*' || key == '#') {
        // Reset input
        heightDisplayed = 0;
        heightIndex = 0;
        LCD_clearDisplay();
        LCD_printString("Cleared");
				lastKeyPressed = '\0';
        delay(500);
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
											lastKeyPressed = '\0';
										}
										else if (key == '2'){
											LCD_clearDisplay();
											LCD_printString("Begin Manual");
											delay(1000);
											currentState =  FILL_MANUAL;
											selectDisplayed = 0;
											lastKeyPressed = '\0';
										}
										else if (key == '*'){
											currentState = RESETTER;
											lastKeyPressed = '\0';
										}
										else if (key == '#'){
											currentState = RESETTER;
											lastKeyPressed = '\0';
										}
										else if (key != '\0'){
											LCD_clearDisplay();
											LCD_printString("Invalid");
											delay(1000);
											selectDisplayed=0;
											lastKeyPressed = '\0';
										}
                break;
									}

						case FILL_AUTO:
{
    trigger_ultrasonic();
    delay(5);

    float measured = distancecalc_avg(15); // update if captured
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
    static char fillBuffer[5] = {0};   // Stores "x.x"
    static int fillIndex = 0;
    static float targetFill = 0;
    static uint8_t promptDisplayed = 0;
    static float emptyDistance = 0;
    static uint8_t pumping = 0;

    if (!promptDisplayed) {
        LCD_clearDisplay();
				LCD_printString("Inches to Fill:");
        LCD_placeCursorRC(2, 0);
        fillIndex = 0;
        fillBuffer[0] = '\0';
        promptDisplayed = 1;
    }

    // Handle number input (auto decimal)
    if (promptDisplayed == 1 && key >= '0' && key <= '9') {
        if (fillIndex == 0) {
            fillBuffer[fillIndex++] = key;
            fillBuffer[fillIndex++] = '.'; // insert decimal
            LCD_printChar(key);
            LCD_printChar('.');
						lastKeyPressed = '\0';
        }
        else if (fillIndex == 2) {
            fillBuffer[fillIndex++] = key;
            fillBuffer[fillIndex] = '\0';
            LCD_printChar(key);
						lastKeyPressed = '\0';

            float requestedFill = atof(fillBuffer);
            if (requestedFill > cupHeight || requestedFill < 0.1f) {
                LCD_clearDisplay();
                LCD_printString("Invalid amount");
                delay(1000);
                promptDisplayed = 0; // reset input
            } else {
                targetFill = requestedFill;
							
                promptDisplayed = 2;
                LCD_clearDisplay();
                LCD_printString("Measuring...");
                emptyDistance = distancecalc_avg(20);  // Get baseline distance
							  delay(500);

                if (emptyDistance <= 0 || emptyDistance > 20.0f) {
                    LCD_clearDisplay();
                    LCD_printString("Measure Failed");
                    delay(1000);
                    promptDisplayed = 0;
                    break;
                }
            }
        }
    }
else if (key == '*') {
    LCD_clearDisplay();
    LCD_printString("Back");
    delay(1000);
    promptDisplayed = 0;
		targetFill = 0;
		emptyDistance = 0;
		pump_stop();
		pumping=0;
    fillIndex = 0;
		lastKeyPressed = '\0';
}
else if (key == '#') {
    LCD_clearDisplay();
    promptDisplayed = 0;
    fillIndex = 0;
    pumping = 0;
		pump_stop();
		currentState = RESETTER;
}


    // Measurement and pumping loop
    if (promptDisplayed == 2) {
        if (key == '*' || key == '#') {
            pump_stop();
            LCD_clearDisplay();
            LCD_printString("Cancelled");
            delay(1000);
            currentState = DONE;
            promptDisplayed = 0;
            pumping = 0;
            break;
        }

        float currentDistance = distancecalc_avg(20);
        float poured = emptyDistance - currentDistance;
        if (poured < 0) poured = 0;

        LCD_clearDisplay();
        LCD_placeCursorRC(1, 0);
        sprintf(buffer, "Target: %.2f", targetFill);
        LCD_printString(buffer);

        LCD_placeCursorRC(2, 0);
        sprintf(buffer, "Poured: %.2f", poured);
        LCD_printString(buffer);

        if (poured < targetFill) {
            pump_run();
            pumping = 1;
        } else {
            if (pumping) {
                pump_stop();
                pumping = 0;
                LCD_clearDisplay();
                LCD_printString("Fill Done!");
                delay(1000);
                currentState = DONE;
                promptDisplayed = 0;
            }
        }

        delay(200); // update rate
    }

    break;
}

           case DONE:
{
    float temp = 0;

    LCD_clearDisplay();
    LCD_printString("Enjoy Your Drink!");
    LCD_placeCursorRC(2, 0);
		LCD_printString("Temp: ");
    LCD_printFloat(temp, 1);
    LCD_printString(" F");

    delay(1000); // update every second

    if (lastKeyPressed != '\0') {
        lastKeyPressed = '\0';
        currentState = SELECT;
    }

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
							lastKeyPressed = '\0';
							selectDisplayed = 0;
							heightDisplayed = 0;
							currentState = SELECT;
							break;
						}
						

            default:
							currentState=SELECT;
                break;
        }

    delay(5);
}
}


void timer2_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable TIM2 clock

    TIM2->PSC = 1599;       // Prescaler: 16MHz / (1599 + 1) = 10 kHz
    TIM2->ARR = 199;        // ARR: 10kHz / (199 + 1) = 50 Hz = 20ms
    TIM2->DIER |= TIM_DIER_UIE; // Enable update interrupt
    TIM2->CR1 |= TIM_CR1_CEN;   // Start timer

    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;

        char key = keypad_scan();
        if (key != '\0') {
            lastKeyPressed = key;
        }
    }
}
