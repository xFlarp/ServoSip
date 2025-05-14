#include "lcd.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include "ultra.h"
#include "pump.h"
#include "temp.h"
#include "keypad.h"

void timer2_init(void);

enum STATE { INPUT_HEIGHT, SELECT, FILL_AUTO, FILL_MANUAL, DONE, RESETTER, TESTER }; 
enum STATE currentState = INPUT_HEIGHT;

extern uint8_t captureDone;
char buffer[16];
char buffer2[16];

char heightBuffer[4];
int heightIndex = 0;
float cupHeight = 0;

extern volatile char lastKeyPressed;

int heightDisplayed = 0;
int selectDisplayed = 0;

static char fillBuffer[5] = {0};
static int fillIndex = 0;
static float targetFill = 0;
static float emptyDistance = 0;
static uint8_t promptStage = 0;

int main (void) {
    LCD_port_init();
    timer2_init();
    LCD_init();
    pwm_init();
    incap_init();
    keypad_init();
    pump_init();
    I2C_Init();
    lastKeyPressed = '\0';

    LCD_clearDisplay();
    LCD_printString("ServoSip");
    LCD_placeCursor(2);
    LCD_printString("Ben & Sam");
    delay(1000);
    currentState = INPUT_HEIGHT;

    while (1) {
        char key = lastKeyPressed;

        switch (currentState) {

            case INPUT_HEIGHT: {
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

                        heightBuffer[heightIndex] = '\0';
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
                    heightDisplayed = 0;
                    heightIndex = 0;
                    LCD_clearDisplay();
                    LCD_printString("Cleared");
                    lastKeyPressed = '\0';
                    delay(500);
                }

                break;
            }

            case SELECT: {
                if (!selectDisplayed) {
                    LCD_clearDisplay();
                    LCD_printString("Select Mode:");
                    LCD_placeCursorRC(2, 0);
                    LCD_printString("1.Auto 2.Manual");
                    selectDisplayed = 1;
                }

                if (key == '1') {
                    LCD_clearDisplay();
                    LCD_printString("Begin Auto");
                    delay(1000);
                    currentState = FILL_AUTO;
                    selectDisplayed = 0;
                    lastKeyPressed = '\0';
                }
                else if (key == '2') {
                    LCD_clearDisplay();
                    LCD_printString("Begin Manual");
                    delay(500);
                    currentState = FILL_MANUAL;
                    selectDisplayed = 0;
                    lastKeyPressed = '\0';
                }
                else if (key == '3') {
                    LCD_clearDisplay();
                    LCD_printString("DEBUG MODE");
                    delay(400);
                    currentState = TESTER;
                    lastKeyPressed = '\0';
                }
                else if (key == '*') {
                    currentState = INPUT_HEIGHT;
                    selectDisplayed = 0;
                    lastKeyPressed = '\0';
                }
                else if (key == '#') {
                    currentState = RESETTER;
                    lastKeyPressed = '\0';
                }
                else if (key != '\0') {
                    LCD_clearDisplay();
                    LCD_printString("Invalid");
                    delay(1000);
                    selectDisplayed = 0;
                    lastKeyPressed = '\0';
                }

                break;
            }

            case FILL_AUTO: {
                #define FULL_CUP_DISTANCE 4.4f
                if (promptStage == 0) {
                    LCD_clearDisplay();
                    LCD_printString("Measuring...");
                    delay(500);
                    emptyDistance = distancecalc_avg(30);
                    promptStage = 1;
                    break;
                }

                if (promptStage == 1) {
                    if (key == '*') {
                        pump_stop();
                        promptStage = 0;
                        lastKeyPressed = '\0';
                        currentState = SELECT;
                        break;
                    }
                    else if (key == '#') {
                        pump_stop();
                        lastKeyPressed = '\0';
                        currentState = RESETTER;
                        break;
                    }

                    float currentDistance = distancecalc();
                    float poured = emptyDistance - currentDistance;
                    if (poured < 0) poured = 0;

                    LCD_clearDisplay();
                    sprintf(buffer, "E:%.1f C:%.1f", emptyDistance, currentDistance);
                    LCD_printString(buffer);
                    LCD_placeCursorRC(2, 0);
                    sprintf(buffer, "P:%.1f T:%.1f", poured, emptyDistance - FULL_CUP_DISTANCE);
                    LCD_printString(buffer);
                    delay(200);

                    if ((currentDistance < FULL_CUP_DISTANCE + 0.1f) && (!(currentDistance > 12.0f))) {
                        pump_stop();
                        LCD_clearDisplay();
                        LCD_printString("Fill Done!");
                        lastKeyPressed = '\0';
                        currentState = DONE;
                        delay(400);
                    }
                    else {
                        pump_run();
                    }
                }

                break;
            }

            case FILL_MANUAL: {
                if (promptStage == 0) {
                    LCD_clearDisplay();
                    LCD_printString("Inches to Fill:");
                    LCD_placeCursorRC(2, 0);
                    fillIndex = 0;
                    fillBuffer[0] = '\0';
                    promptStage = 1;
                    break;
                }

                if (promptStage == 1) {
                    if (key >= '0' && key <= '9') {
                        if (fillIndex == 0) {
                            fillBuffer[fillIndex++] = key;
                            fillBuffer[fillIndex++] = '.';
                            LCD_printChar(key);
                            LCD_printChar('.');
                            lastKeyPressed = '\0';
                        }
                        else if (fillIndex == 2) {
                            fillBuffer[fillIndex++] = key;
                            fillBuffer[fillIndex] = '\0';
                            LCD_printChar(key);
                            lastKeyPressed = '\0';

                            float requested = atof(fillBuffer);
                            if (requested < 0.1f || requested > cupHeight) {
                                LCD_clearDisplay();
                                LCD_printString("Invalid Input");
                                delay(1000);
                                promptStage = 0;
                                break;
                            }

                            targetFill = requested;
														LCD_clearDisplay();
														LCD_printString("Measuring...");
														delay(500);
														emptyDistance = distancecalc_avg(35);

														float projectedDistance = emptyDistance - targetFill;
															if (projectedDistance < FULL_CUP_DISTANCE) {
																	LCD_clearDisplay();
																	LCD_printString("Overflow Risk!");
																	delay(1500);
																	promptStage = 0;
																	break;
															}

														promptStage = 2;
														break;

                        }
                    }

                    if (key == '*' || key == '#') {
                        promptStage = 0;
                        currentState = SELECT;
                        lastKeyPressed = '\0';
                        break;
                    }
                }

                if (promptStage == 2) {
                    if (key == '*') {
                        pump_stop();
                        promptStage = 0;
                        lastKeyPressed = '\0';
                        currentState = SELECT;
                        break;
                    }
                    else if (key == '#') {
                        pump_stop();
                        lastKeyPressed = '\0';
                        currentState = RESETTER;
                        break;
                    }

                    float currentDistance = distancecalc();
                    float poured = emptyDistance - currentDistance;
                    if (poured < 0) poured = 0;

                    LCD_clearDisplay();
                    sprintf(buffer, "E:%.1f C:%.1f", emptyDistance, currentDistance);
                    LCD_printString(buffer);
                    LCD_placeCursorRC(2, 0);
                    sprintf(buffer, "P:%.1f T:%.1f", poured, targetFill);
                    LCD_printString(buffer);
                    delay(200);

                    if (poured + 0.1f < targetFill) {
                        pump_run();
                    }
                    else {
                        pump_stop();
                        LCD_clearDisplay();
                        LCD_printString("Fill Done!");
                        lastKeyPressed = '\0';
                        currentState = DONE;
                        delay(400);
                        break;
                    }
                }

                break;
            }

            case DONE: {
                float temp = TEMP();
                LCD_clearDisplay();
                LCD_printString("Enjoy Your Drink!");
                LCD_placeCursorRC(2, 0);
                LCD_printString("Temp: ");
                LCD_printFloat(temp, 1);
                LCD_printString(" F");

                delay(1000);

                if (lastKeyPressed != '\0') {
                    lastKeyPressed = '\0';
                    heightDisplayed = 0;
                    selectDisplayed = 0;
                    fillIndex = 0;
                    fillBuffer[0] = '\0';
                    targetFill = 0;
                    promptStage = 0;
                    emptyDistance = 0;
                    currentState = SELECT;
                }

                break;
            }

            case RESETTER: {
                LCD_clearDisplay();
                LCD_printString("RESET");
                delay(500);
                LCD_clearDisplay();
                cupHeight = 0;
                heightIndex = 0;
                lastKeyPressed = '\0';
                selectDisplayed = 0;
                promptStage = 0;
                heightDisplayed = 0;
                currentState = INPUT_HEIGHT;
                break;
            }

            case TESTER: {
                float distance = distancecalc_avg(15);
                float temp = TEMP();
                LCD_clearDisplay();
                snprintf(buffer, sizeof(buffer), "Dist: %.1f in", distance);
                LCD_printString(buffer);
                LCD_placeCursor(2);
                sprintf(buffer2, "Temp: %.1f F", temp);
                LCD_printString(buffer2);
                delay(100);

                if (lastKeyPressed != '\0') {
                    lastKeyPressed = '\0';
                    selectDisplayed = 0;
                    currentState = SELECT;
                }

                break;
            }

            default:
                currentState = SELECT;
                break;
        }

        delay(5);
    }
}

void timer2_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 1599;
    TIM2->ARR = 199;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

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
