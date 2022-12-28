#include "neopixels.h"
#include "serial.h"
#include "sysTick.h"
#include "../STM32F446RE/stm32f446xx.h"
#include <stdio.h>

#define PA5_AF_MODE (1 << 11)
#define PA5_AF1     (1 << 20)
#define CH1_PWM_MODE_1 (6 << 4)

#define TIMER_PRESCALER 2
#define WS2821_FREQUENCY_800KHZ_TICKS 56
#define LOGIC_0_TICKS 16
#define LOGIC_1_TICKS 32

#define DMA_CHANNEL_3 (3 <<25)
#define DMA_MEM_TO_PERIPHERAL ( 1 << 6)
#define MEM_SIZE_32 (1 << 14)
#define PERIPH_SIZE_32 ( 1 << 12)

void timer_init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    GPIOA->MODER |= PA5_AF_MODE;
    GPIOA->AFR[0] |= PA5_AF1;

    TIM2->PSC = TIMER_PRESCALER-1;
    TIM2->ARR = WS2821_FREQUENCY_800KHZ_TICKS;
    TIM2->CCMR1 = CH1_PWM_MODE_1;
    TIM2->CCR1 = 0;

    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->DIER |= TIM_DIER_CC1IE;
    TIM2->DIER |= TIM_DIER_CC1DE;
    TIM2->DIER |= TIM_DIER_UDE;
     TIM2->CR2 |= TIM_CR2_CCDS;

    TIM2->CCER |= TIM_CCER_CC1E; 
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM2_IRQn);
}

void dma_init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    DMA1_Stream5->CR = 0;
    while(DMA1_Stream5->CR & DMA_SxCR_EN);

    DMA1_Stream5->CR |= DMA_CHANNEL_3;
    // DMA1_Stream5->CR |= DMA_SxCR_CIRC;
    DMA1_Stream5->CR |= DMA_SxCR_MINC;
    DMA1_Stream5->CR |= MEM_SIZE_32;
    DMA1_Stream5->CR |= PERIPH_SIZE_32;
    DMA1_Stream5->CR |= DMA_MEM_TO_PERIPHERAL;
    DMA1_Stream5->CR |= DMA_SxCR_TCIE;

    NVIC_EnableIRQ(DMA1_Stream5_IRQn);
};

Led setColor(int r, int g, int b){
    Led color = {r, g, b};
    return color;
};

volatile int isDone = 0;

void send(Led *strip, int length){
    volatile  uint32_t color;
    volatile uint32_t buffer[24];
   
    for(int i=0; i<8; i++){   
        while(DMA1_Stream5->CR & DMA_SxCR_EN){};
        color = ((strip[i].g << 16) | (strip[i].r << 8 ) | (strip[i].b));

        for(int j=23; j>=0; j--){
            if(color & (1 << j)) buffer[23-j] = LOGIC_1_TICKS;
            else buffer[23-j] = LOGIC_0_TICKS;
        }   

        DMA1_Stream5->NDTR = 24;
        DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
        DMA1_Stream5->M0AR = (uint32_t)(&buffer);  
        DMA1_Stream5->CR |=  DMA_SxCR_EN;
         while(!isDone);
    }  
    while(!isDone);
    delay_ms(3);
    //  TIM2->CNT = 0;
    //  TIM2->CCR1 = 0;
    TIM2->CCER &=~ TIM_CCER_CC1E; 
    TIM2->CR1 &=~ TIM_CR1_CEN;
    delay_ms(2);
    TIM2->CCER |= TIM_CCER_CC1E; 
    TIM2->CR1 |= TIM_CR1_CEN;
     TIM2->CNT = 0;
     TIM2->CCR1 = 0;
delay_ms(2);
}   

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &=~ TIM_SR_UIF;
        if(isDone) {       
            TIM2->CCR1 = 0;
            isDone = 0;
        }
        
    }
    else if(TIM2->SR & TIM_SR_CC1IF){
        TIM2->SR &=~ TIM_SR_CC1IF;
        
    }
}

void DMA1_Stream5_IRQHandler(void){
    if(DMA1->HISR & DMA_HISR_TCIF5){
        DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
        isDone = 1;
        
    }
}

int __io_putchar(int ch){
    tx_send(ch);
    return ch;
}


// #include "neopixels.h"
// #include "serial.h"
// #include "../STM32F446RE/stm32f446xx.h"

// #define PA5_AF_MODE (1 << 11)
// #define PA5_AF1     (1 << 20)
// #define CH1_PWM_MODE_1 (6 << 4)

// #define TIMER_PRESCALER 2
// #define WS2821_FREQUENCY_800KHZ_TICKS 56
// #define LOGIC_0_TICKS 16
// #define LOGIC_1_TICKS 32

// #define DMA_CHANNEL_3 (3 <<25)
// #define DMA_MEM_TO_PERIPHERAL ( 1 << 6)
// #define MEM_SIZE_32 (1 << 14)
// #define PERIPH_SIZE_32 ( 1 << 12)

// volatile int intervals[] = {
//     LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//     LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS ,LOGIC_1_TICKS , LOGIC_1_TICKS,
//     LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
    
//     LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS ,LOGIC_1_TICKS , LOGIC_1_TICKS,
//     LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//     LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
     
//      LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//       LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//        LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
           
//      LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//       LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//        LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
           
//      LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//       LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//        LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,

//     LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//     LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS, LOGIC_0_TICKS ,LOGIC_0_TICKS , LOGIC_0_TICKS,
//     LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS, LOGIC_1_TICKS ,LOGIC_1_TICKS , LOGIC_0_TICKS,

// };
// volatile int counter = 0;

// void timer_init(){
//     // __disable_irq();
//     RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
//     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
//     GPIOA->MODER |= PA5_AF_MODE;
//     GPIOA->AFR[0] |= PA5_AF1;

//     TIM2->PSC = TIMER_PRESCALER - 1;
//     TIM2->ARR = WS2821_FREQUENCY_800KHZ_TICKS;
//     TIM2->CCMR1 = CH1_PWM_MODE_1;
//     TIM2->CCR1 = 0;

//     TIM2->DIER |= TIM_DIER_UIE;
//     TIM2->DIER |= TIM_DIER_CC1IE;
//     TIM2->DIER |= TIM_DIER_CC1DE;
//     TIM2->DIER |= TIM_DIER_UDE;
//     TIM2->CR2 |= TIM_CR2_CCDS;

//     TIM2->CCER |= TIM_CCER_CC1E; 
//     TIM2->CR1 |= TIM_CR1_CEN;

//     NVIC_EnableIRQ(TIM2_IRQn);
//     // __enable_irq();
// }

// void dma_init(){
//     RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
//     DMA1_Stream5->CR = 0;
//     while(DMA1_Stream5->CR & DMA_SxCR_EN);

//     DMA1_Stream5->CR |= DMA_CHANNEL_3;
//     // DMA1_Stream5->CR |= DMA_SxCR_CIRC;
//     DMA1_Stream5->CR |= DMA_SxCR_MINC;
//     DMA1_Stream5->CR |= MEM_SIZE_32;
//     DMA1_Stream5->CR |= PERIPH_SIZE_32;
//     DMA1_Stream5->CR |= DMA_MEM_TO_PERIPHERAL;
//     DMA1_Stream5->CR |= DMA_SxCR_TCIE;

//     DMA1_Stream5->NDTR = 144;
//     DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
//     DMA1_Stream5->M0AR = (uint32_t)(&intervals);  

//     NVIC_EnableIRQ(DMA1_Stream5_IRQn);

//     DMA1_Stream5->CR |=  DMA_SxCR_EN;
// };

// Led setColor(short r, short g, short b){
//     Led color = {g, r, b};
//     return color;
// };

// void send(Led* strip, int length){

// }

// void TIM2_IRQHandler(void) {
//     if (TIM2->SR & TIM_SR_UIF) {
//         TIM2->SR &=~ TIM_SR_UIF;
//         //printf("UIF  %d\n",TIM2->CCR1);
    
//     }
//     else if(TIM2->SR & TIM_SR_CC1IF){
//         TIM2->SR &=~ TIM_SR_CC1IF;
//        // printf("CC1IF %d\n",TIM2->CCR1);
//     }
// }

// void DMA1_Stream5_IRQHandler(void){
//     if(DMA1->HISR & DMA_HISR_TCIF5){
//         DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
//            TIM2->CCR1 = 0;
//         //   TIM2->CR1 &=~ TIM_CR1_CEN;
//         //  DMA1_Stream5->CR &=~  DMA_SxCR_EN;
//         //printf("DMA DONE %d\n",TIM2->CCR1);
//     }
    
// }

// int __io_putchar(int ch){
//     tx_send(ch);
//     return ch;
// }