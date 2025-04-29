#include "stm32f4xx.h"

void keypad_init(void);
char keypad_scan(void);
void handleKeyPress(void);
void processKey(char key);

extern volatile char lastKeyPressed;