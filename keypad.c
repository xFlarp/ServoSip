#include "keypad.h"
#include "gpio.h"
#include "lcd.h"

volatile char lastKeyPressed = '\0';

static const char keys[4][3] = {
    {'3', '2', '1'},
    {'6', '5', '7'},
    {'9', '8', '7'},
    {'#', '0', '*'}
};

// Tracks if a key is currently being held
static char keyLock = '\0';

void keypad_init(void) {
    // Enable clock for GPIOE
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // Set PE12–PE15 (rows) as output
    GPIO_setOutput(GPIOE, 12);
    GPIO_setOutput(GPIOE, 13);
    GPIO_setOutput(GPIOE, 14);
    GPIO_setOutput(GPIOE, 15);

    // Set PE9–PE11 (columns) as input with pull-up
    GPIO_setInput(GPIOE, 9);
    GPIO_setInput(GPIOE, 10);
    GPIO_setInput(GPIOE, 11);

    GPIO_setPullResistor(GPIOE, 9, 1);  // Pull-up
    GPIO_setPullResistor(GPIOE, 10, 1);
    GPIO_setPullResistor(GPIOE, 11, 1);

    // Set all rows HIGH initially
    GPIOE->ODR |= (0xF << 12);
}

char keypad_scan(void) {
    for (int row = 0; row < 4; row++) {
        // Set all rows HIGH
        GPIOE->ODR |= (0xF << 12);

        // Pull current row LOW
        GPIOE->ODR &= ~(1 << (15 - row));

        delay(1); // Short delay to stabilize line

        for (int col = 0; col < 3; col++) {
            //int pin = 11 - col;  // PE11, PE10, PE9
							int pin = 9 + col;
					
            if (!(GPIOE->IDR & (1 << pin))) {  // Active low
                char detected = keys[row][col];

                if (keyLock == '\0') {
                    keyLock = detected;  // Latch key
                    return detected;
                } else {
                    return '\0'; // Already held, wait for release
                }
            }
        }
    }

    // If no key is pressed, clear lock
    keyLock = '\0';
    return '\0';
}
