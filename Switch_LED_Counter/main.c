#include "stm32l476xx.h"

// PA5  <--> Green LED

#define PB4     4	//LED1
#define PB5		5	//LED2
#define	PC2		2	//SW1
#define PC3		3	//SW2

volatile uint8_t debounce_SW1 = 0;
volatile uint8_t debounce_SW2 = 0;

volatile uint8_t counter = 0;  // 2-bit counter: values 0–3

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

void update_LEDs_from_counter() {
    if (counter & 0x01)
        turn_on_LED1();
    else
        turn_off_LED1();

    if (counter & 0x02)
        turn_on_LED2();
    else
        turn_off_LED2();
}

void TIM6_Init(uint16_t ms_delay) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;  // Enable TIM6 clock

    TIM6->CR1 = 0;               // Clear control register
    TIM6->PSC = 3999;            // Prescaler: (4 MHz / (3999 + 1)) = 1 kHz → 1ms tick
    TIM6->ARR = ms_delay;        // Auto-reload value: e.g., 20 ms

    TIM6->DIER |= TIM_DIER_UIE;  // Enable update interrupt
    NVIC_EnableIRQ(TIM6_DAC_IRQn); // Enable TIM6 IRQ
}

void TIM6_DAC_IRQHandler(void) {
    TIM6->SR &= ~TIM_SR_UIF;     // Clear interrupt flag
    TIM6->CR1 &= ~TIM_CR1_CEN;   // Stop timer

    if (debounce_SW1) {
        debounce_SW1 = 0;
        if ((GPIOC->IDR & (1 << PC2)) != 0) {  // SW1 pressed = HIGH (pull-down)
            counter = (counter + 1) & 0x03;    // increment & wrap (0–3)
            update_LEDs_from_counter();
        }
        EXTI->IMR1 |= (1 << PC2);
    }

    if (debounce_SW2) {
        debounce_SW2 = 0;
        if ((GPIOC->IDR & (1 << PC3)) == 0) {  // SW2 pressed = LOW (pull-up)
            counter = (counter - 1) & 0x03;    // decrement & wrap (0–3)
            update_LEDs_from_counter();
        }
        EXTI->IMR1 |= (1 << PC3);
    }
}

// === EXTI CONFIG ===
void configure_EXTI() {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Enable IRQ for PC2 and PC3
    NVIC_EnableIRQ(EXTI2_IRQn);   // SW1
    NVIC_EnableIRQ(EXTI3_IRQn);   // SW2

    // Map EXTI2 to PC2, EXTI3 to PC3
    SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI2 | SYSCFG_EXTICR1_EXTI3);
    SYSCFG->EXTICR[0] |=  (SYSCFG_EXTICR1_EXTI2_PC | SYSCFG_EXTICR1_EXTI3_PC);

    // Unmask EXTI lines
    EXTI->IMR1 |= (1 << PC2) | (1 << PC3);

    // SW1 (PC2) — rising edge
    EXTI->RTSR1 |= (1 << PC2);
    EXTI->FTSR1 &= ~(1 << PC2);

    // SW2 (PC3) — falling edge
    EXTI->FTSR1 |= (1 << PC3);
    EXTI->RTSR1 &= ~(1 << PC3);
}

// === INTERRUPT SERVICE ROUTINES ===
void EXTI2_IRQHandler(void) {  // SW1 - PC2 (pull-down)
    if (EXTI->PR1 & (1 << PC2)) {
        EXTI->PR1 |= (1 << PC2);           // Clear flag
        EXTI->IMR1 &= ~(1 << PC2);         // Disable EXTI for PC2
        debounce_SW1 = 1;                  // Set debounce flag
        TIM6_Init(20);                     // 20 ms debounce
        TIM6->CR1 |= TIM_CR1_CEN;          // Start timer
    }
}

void EXTI3_IRQHandler(void) {  // SW2 - PC3 (pull-up)
    if (EXTI->PR1 & (1 << PC3)) {
        EXTI->PR1 |= (1 << PC3);           // Clear flag
        EXTI->IMR1 &= ~(1 << PC3);         // Disable EXTI for PC3
        debounce_SW2 = 1;                  // Set debounce flag
        TIM6_Init(20);                     // 20 ms debounce
        TIM6->CR1 |= TIM_CR1_CEN;          // Start timer
    }
}

int main(void){
	configure_LED_pin();
    configure_Push_Button_pin();
    configure_EXTI();

    counter = 0;
    update_LEDs_from_counter();

    while (1) {
        __WFI();  // Wait For Interrupt — saves power
    }
		
}
