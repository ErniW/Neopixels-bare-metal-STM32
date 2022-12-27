#include "../STM32F446RE/stm32f446xx.h"

#include "neopixels.h"
#include "pll.h"
#include "sysTick.h"

#define TIMER_PRESCALER 2
#define TIMER_FREQUENCY ( (APB1_FREQ * 2) / TIMER_PRESCALER)

#define WS2821_FREQUENCY_800KHZ_TICKS 56
#define LOGIC_0_TICKS 16
#define LOGIC_1_TICKS 32

#define PA5_AF_MODE (1 << 11)
#define PA5_AF1     (1 << 20)
#define CH1_PWM_MODE (6 << 4)

#define DMA_CHANNEL_3 (3 <<25)
#define MEM_SIZE_32 (1 << 14)
#define PERIPH_SIZE_32 ( 1 << 12)

#define DMA_MEM_TO_PERIPHERAL ( 1 << 6)


void timer_init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    GPIOA->MODER |= PA5_AF_MODE;
    GPIOA->AFR[0] |= PA5_AF1;

     TIM2->PSC = TIMER_PRESCALER - 1;
    TIM2->ARR =  WS2821_FREQUENCY_800KHZ_TICKS;
    TIM2->CNT = 0;

    TIM2->CCMR1 |= CH1_PWM_MODE;

    TIM2->DIER |= TIM_DIER_CC1DE;
    TIM2->DIER |= TIM_DIER_CC1IE;
     TIM2->DIER |= TIM_DIER_UDE;
    TIM2->DIER |= TIM_DIER_TDE;
    TIM2->CR1 |= TIM_CR1_URS;

    TIM2->CCR1 = 0;
     TIM2->CCER |= TIM_CCER_CC1E; 
    TIM2->CR1 |= TIM_CR1_CEN;
     NVIC_EnableIRQ(TIM2_IRQn);
}

uint32_t buffer[24];

void dma_init(int vals, int peripheral, int size){
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    DMA1_Stream5->CR = 0;
    while(DMA1_Stream5->CR & DMA_SxCR_EN);

    DMA1_Stream5->CR |= DMA_CHANNEL_3;
    DMA1_Stream5->CR |= DMA_SxCR_CIRC;
    DMA1_Stream5->CR |= DMA_SxCR_MINC;
    DMA1_Stream5->CR |= MEM_SIZE_32;
    DMA1_Stream5->CR |= PERIPH_SIZE_32;
    DMA1_Stream5->CR |= DMA_MEM_TO_PERIPHERAL;

    // DMA1_Stream5->NDTR = sizeof(vals)/sizeof(uint32_t);
    // DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
    // DMA1_Stream5->M0AR = (uint32_t)(&vals);

    DMA1_Stream5->NDTR = 24;
    DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
    DMA1_Stream5->M0AR = (uint32_t)(&buffer);    
    
      DMA1_Stream5->CR |=  DMA_SxCR_EN;
      TIM2->SR &=~ TIM_SR_UIF;
}

Led setColor(short r, short g, short b){
    Led color = {g, r, b};
    return color;
};



void send(Led* strip, int length){
    uint32_t color;
    for(int led=0; led<length; led++){

          TIM2->CR1 |= TIM_CR1_CEN;
        color = ((strip[led].g << 16) | (strip[led].r << 8 ) | (strip[led].b));
    
        for(int i=23; i >=0; i--){
            if(color & (1 << i)) buffer[i] = LOGIC_0_TICKS;
            else buffer[i] = LOGIC_1_TICKS;

             printf("%d\n", buffer[i]);
        }

        
        
        
        //  DMA1_Stream5->CR |=  DMA_SxCR_EN;   
        // DMA1_Stream5->CR |=  DMA_SxCR_EN;
         
        DMA1->HISR |= DMA_HISR_TCIF5;
        printf("%ld\n", color);
        // TIM2->CNT = 0;
        // TIM2->CR1 |= TIM_CR1_CEN;
         DMA1_Stream5->CR |=  DMA_SxCR_EN;
        while(!(DMA1->HISR & (DMA_HISR_TCIF5)));
        DMA1->HISR |= DMA_HISR_TCIF5;
        // // TIM2->CR1 &=~ TIM_CR1_CEN;
        // while(!(DMA1->HISR & DMA_HISR_TCIF5));
        // DMA1->HISR &=~ DMA_HISR_TCIF5;
    }
//     TIM2->CR1 &=~ TIM_CR1_CEN;
//    while(!(DMA1->HISR & DMA_HISR_TCIF5));
//    TIM2->CR1 &=~ TIM_CR1_CEN;
    //delay_ms(5);
    
    //delay_ms(5);
}



void stop(){

}

void TIM2_IRQHandler(void) {
    if(TIM2->SR & TIM_SR_UIF){
         TIM2->SR &=~ TIM_SR_UIF;
        //  printf("%d\n", TIM2->CNT);
        
    }
    else if(TIM2->SR & TIM_SR_CC1OF){
        TIM2->SR &=~ TIM_SR_CC1OF;
    }
}