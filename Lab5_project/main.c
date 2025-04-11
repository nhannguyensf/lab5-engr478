#include "stm32l476xx.h"

// PA5  <--> Green LED

#define PB4   4	//LED1
#define PB5		5	//LED2
#define	PC2		2	//SW1
#define PC3		3	//SW2

volatile uint8_t toggle_flag = 0;

void configure_LED_pin(){
  // 1. Enable the clock to GPIO Port B	
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;   
		
	// 2. Configure GPIO Mode to 'Output': Input(00), Output(01), AlterFunc(10), Analog(11)
	GPIOB->MODER &= ~(3UL<<(2*PB4));  
	GPIOB->MODER |=   1UL<<(2*PB4);      // Output(01)
	GPIOB->MODER &= ~(3UL<<(2*PB5));  
	GPIOB->MODER |=   1UL<<(2*PB5);      // Output(01)
	
	// 3. Configure GPIO Output Type to 'Push-Pull': Output push-pull (0), Output open drain (1) 
	GPIOB->OTYPER &= ~(1<<PB4);      // Push-pull
	GPIOB->OTYPER &= ~(1<<PB5);      // Push-pull
	// 4. Configure GPIO Push-Pull to 'No Pull-up or Pull-down': No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOB->PUPDR  &= ~(3<<(2*PB4));  // No pull-up, no pull-down
	GPIOB->PUPDR  &= ~(3<<(2*PB5));  // No pull-up, no pull-down
}

void configure_Push_Button_pin(){
  // 1. Enable the clock to GPIO Port A	
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;   
		
	// 2. Configure GPIO Mode to 'Input': Input(00), Output(01), AlterFunc(10), Analog(11)
	GPIOC->MODER &= ~(3UL<<(2*PC2));  	// Input (00)
	GPIOC->MODER &= ~(3UL<<(2*PC3));  	// Input (00)
	
	// 3. Configure GPIO Push-Pull to 'No Pull-up or Pull-down': No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)
	GPIOC->PUPDR  &= ~(3<<(2*PC2));  // No pull-up, no pull-down, clear bits
	GPIOC->PUPDR |=  (2 << (2 * PC2));   // Pull-down (10)

	GPIOC->PUPDR  &= ~(3<<(2*PC3));  // No pull-up, no pull-down, clear bits
	GPIOC->PUPDR |=  (1 << (2 * PC3));   // Pull-up (01)
}

// Modular function to turn on the LD2 LED.
void turn_on_LED1(){
	GPIOB->ODR |= 1 << PB4;
}
void turn_on_LED2(){
	GPIOB->ODR &= ~(1 << PB5);
}

// Modular function to turn off the LD2 LED.
void turn_off_LED1(){
	GPIOB->ODR &= ~(1 << PB4);
}
void turn_off_LED2(){
	GPIOB->ODR |= 1 << PB5;
}

// Modular function to toggle the LD2 LED.
void toggle_LED1(){
	GPIOB->ODR ^= (1 << PB4);
}
void toggle_LED2(){
	GPIOB->ODR ^= (1 << PB5);
}

void SysTick_Init(uint32_t reload_val) {
    SysTick->CTRL = 0;                  // Disable SysTick
    SysTick->LOAD = reload_val - 1;     // Set reload value
    SysTick->VAL = 0;                   // Reset current value
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | 
                     SysTick_CTRL_TICKINT_Msk   | 
                     SysTick_CTRL_ENABLE_Msk;   // Enable SysTick with interrupt
}

// Fires every 0.5 second
void SysTick_Handler(void) {
    toggle_flag = 1;
}

int main(void) {
    // Configure PB4 and PB5 as outputs for LED1 and LED2
    configure_LED_pin();

    // Configure PC2 and PC3 as inputs for SW1 and SW2 (with pull-down and pull-up)
    configure_Push_Button_pin();

    // Initialize SysTick timer to trigger interrupt every 0.5 seconds (assuming 4 MHz clock)
    SysTick_Init(2000000); // 4,000,000 cycles/sec × 0.5s = 2,000,000 ticks

    // Main loop
    while (1) {
        // Check if SysTick interrupt has set the toggle_flag
        if (toggle_flag) {
            toggle_flag = 0; // Clear the flag after handling

            // Check if SW2 is pressed (active-low: reads 0 when pressed)
            if ((GPIOC->IDR & (1 << PC3)) == 0) {
                // If SW2 is pressed, turn both LEDs OFF immediately
                turn_off_LED1();
                turn_off_LED2();
            }
            // Else if SW1 is pressed (active-high: reads 1 when pressed)
            else if ((GPIOC->IDR & (1 << PC2)) != 0) {
                // Toggle LED1 every 0.5s and keep LED2 OFF
                toggle_LED1();
                turn_off_LED2();
            }
            // Else (SW1 is not pressed, SW2 is not pressed)
            else {
                // Toggle LED2 every 0.5s and keep LED1 OFF
                toggle_LED2();
                turn_off_LED1();
            }
        }
    }
}
