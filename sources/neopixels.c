#include "neopixels.h"
#include "serial.h"
#include "sysTick.h"
#include "../STM32F446RE/stm32f446xx.h"
#include <stdio.h>

#define PA5_AF_MODE (1 << 11)
#define PA5_AF1     (1 << 20)
#define CH1_PWM_MODE_1 (6 << 4)

// #define TIMER_PRESCALER 2
// #define WS2812_FREQUENCY_800KHZ_TICKS 56
// #define LOGIC_0_TICKS 16
// #define LOGIC_1_TICKS 32
// #define WS2812_RESET_TICKS 2500

#define DMA_CHANNEL_3 (3 <<25)
#define DMA_MEM_TO_PERIPHERAL ( 1 << 6)
#define MEM_SIZE_32 (1 << 14)
#define PERIPH_SIZE_32 ( 1 << 12)

void timer_init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    GPIOA->MODER |= PA5_AF_MODE;
    GPIOA->AFR[0] |= PA5_AF1;

    TIM2->PSC = TIMER_PRESCALER - 1;
    TIM2->ARR = WS2812_FREQUENCY_800KHZ_TICKS;
    TIM2->CCMR1 = CH1_PWM_MODE_1;
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
    DMA1_Stream5->CR |= DMA_SxCR_MINC;
    DMA1_Stream5->CR |= MEM_SIZE_32;
    DMA1_Stream5->CR |= PERIPH_SIZE_32;
    DMA1_Stream5->CR |= DMA_MEM_TO_PERIPHERAL;
    DMA1_Stream5->CR |= DMA_SxCR_TCIE;

    NVIC_EnableIRQ(DMA1_Stream5_IRQn);
};

Led setColor(uint8_t r, uint8_t g, uint8_t b){
    Led color = {r, g, b};
    return color;
};

void send(LedStrip *strip){
    uint32_t color;
    uint32_t buffer[24];
     
   
    while(strip->state != STATE_IDLE){};

    DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
    DMA1_Stream5->M0AR = (uint32_t)(&buffer);
    DMA1_Stream5->NDTR = 24;

    TIM2->CNT = 0;
    TIM2->CCR1 = 0;
    TIM2->CR1 |= TIM_CR1_CEN;

    for(int i=0; i<strip->size; ++i){ 

        color = (strip->led[i].g << 16) | (strip->led[i].r << 8 ) | (strip->led[i].b);

        for(int j=23; j>=0; --j){
            if(color & (1 << j)) buffer[23-j] = LOGIC_1_TICKS;
            else buffer[23-j] = LOGIC_0_TICKS;
        }
          
        DMA1_Stream5->CR |=  DMA_SxCR_EN;
        TIM2->CNT = 0;
        while(strip->state != STATE_PACKET_DONE){};
    }
    
    strip->state = STATE_RESET_START;
}   

// void TIM2_IRQHandler(void) {
//     if (TIM2->SR & TIM_SR_UIF) {
//         TIM2->SR &=~ TIM_SR_UIF;

//         if(isDone) {       
//             TIM2->CCR1 = 0;
//             isDone = 0;
//         }

//         if(isResetting){
//             TIM2->ARR = WS2812_FREQUENCY_800KHZ_TICKS;
//             isResetting = 0;
//         }
//     }
//     else if(TIM2->SR & TIM_SR_CC1IF){
//         TIM2->SR &=~ TIM_SR_CC1IF; 
//     }
// }

// void DMA1_Stream5_IRQHandler(void){
//     if(DMA1->HISR & DMA_HISR_TCIF5){
//         DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
//         isDone = 1;

//         if(isResetting){
//             TIM2->ARR = WS2812_RESET_TICKS;
//         }
//     }
// }

int __io_putchar(int ch){
    tx_send(ch);
    return ch;
}