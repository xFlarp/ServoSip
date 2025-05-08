#include "stm32f4xx.h"

void keypad_init(void);
char keypad_scan(void);
void handleKeyPress(void);

extern volatile char lastKeyPressed;