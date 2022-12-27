 #include "./STM32F446RE/stm32f4xx.h"


#include "./STM32F446RE/stm32f446xx.h"
#include<stdio.h>
#include<stdint.h>
#include "pll.h"
#include "sysTick.h"
#include "serial.h"
#include <string.h>
// #include <iostream>
#include <stdbool.h>

#define PA5_AF_MODE (1 << 11)
#define PA5_AF1     (1 << 20)

volatile int intervals[] = {500, 1000, 2000,3000,4000};
volatile int counter = 0;

inline void startDMA(){
    DMA1_Stream5->CR |=  DMA_SxCR_EN;
}

inline void stopDMA(){
    DMA1_Stream5->CR &=~  DMA_SxCR_EN;
}

int main(){

    clockSpeed_PLL();  
    SysTick_Init();
    tx_init();

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    GPIOA->MODER |= PA5_AF_MODE;
    GPIOA->AFR[0] |= PA5_AF1;

    TIM2->PSC = 9000-1;
    TIM2->ARR = 10000;

    TIM2->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;

    TIM2->CCR1 = 0;
    
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->DIER |= TIM_DIER_CC1IE;

    TIM2->CR1 |= TIM_CR1_URS;

    TIM2->DIER |= TIM_DIER_CC1DE;
    TIM2->DIER |= TIM_DIER_UDE;
    TIM2->CR2 |= TIM_CR2_CCDS;
    
    TIM2->CCER |= TIM_CCER_CC1E; 
    TIM2->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM2_IRQn);
   
    

    //---------------------------------

    #define DMA_CHANNEL_3 (3 <<25)
    #define MEM_SIZE_32 (1 << 14)
    #define PERIPH_SIZE_32 ( 1 << 12)

    #define DMA_MEM_TO_PERIPHERAL ( 1 << 6)


    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    DMA1_Stream5->CR = 0;
    while(DMA1_Stream5->CR & DMA_SxCR_EN);

    DMA1_Stream5->CR |= DMA_CHANNEL_3;
    DMA1_Stream5->CR |= DMA_SxCR_CIRC;
    DMA1_Stream5->CR |= DMA_SxCR_MINC;
    DMA1_Stream5->CR |= MEM_SIZE_32;
    DMA1_Stream5->CR |= PERIPH_SIZE_32;
    DMA1_Stream5->CR |= DMA_MEM_TO_PERIPHERAL;

    DMA1_Stream5->CR |= DMA_SxCR_TCIE;

    DMA1_Stream5->NDTR = 5;
    DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
    DMA1_Stream5->M0AR = (uint32_t)(&intervals);  

    NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    startDMA();

    while(1){

    }

}


void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &=~ TIM_SR_UIF;
        printf("UIF  %d\n",TIM2->CCR1);
    
    }
    else if(TIM2->SR & TIM_SR_CC1IF){
        TIM2->SR &=~ TIM_SR_CC1IF;
        printf("CC1IF %d\n",TIM2->CCR1);
    }
}

void DMA1_Stream5_IRQHandler(void){
    if(DMA1->HISR & DMA_HISR_TCIF5){
        DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
        printf("DMA DONE %d\n",TIM2->CCR1);
    }
    
}


// działający inkrement z każdym cyklem a nie pulsem!

//  #include "./STM32F446RE/stm32f4xx.h"


// #include "./STM32F446RE/stm32f446xx.h"
// #include<stdio.h>
// #include<stdint.h>
// #include "pll.h"
// #include "sysTick.h"
// #include "serial.h"
// #include <string.h>
// // #include <iostream>
// #include <stdbool.h>

// #define PA5_AF_MODE (1 << 11)
// #define PA5_AF1     (1 << 20)

// volatile int intervals[] = {500, 1000, 2000,3000,4000};
// volatile int counter = 0;

// inline void startDMA(){
//     DMA1_Stream5->CR |=  DMA_SxCR_EN;
// }

// inline void stopDMA(){
//     DMA1_Stream5->CR &=~  DMA_SxCR_EN;
// }

// int main(){

//     clockSpeed_PLL();  
//     SysTick_Init();
//     tx_init();

//     RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
//     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

//     GPIOA->MODER |= PA5_AF_MODE;
//     GPIOA->AFR[0] |= PA5_AF1;

//     TIM2->PSC = 9000-1;
//     TIM2->ARR = 10000;

//     TIM2->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;

//     TIM2->CCR1 = 0;
    
//     TIM2->DIER |= TIM_DIER_UIE;
//     TIM2->DIER |= TIM_DIER_CC1IE;

//     TIM2->CR1 |= TIM_CR1_URS;

//     TIM2->DIER |= TIM_DIER_CC1DE;
//     TIM2->DIER |= TIM_DIER_UDE;
//     TIM2->CR2 |= TIM_CR2_CCDS;
    
//     TIM2->CCER |= TIM_CCER_CC1E; 
//     TIM2->CR1 |= TIM_CR1_CEN;

//     NVIC_EnableIRQ(TIM2_IRQn);
   
    

//     //---------------------------------

//     #define DMA_CHANNEL_3 (3 <<25)
//     #define MEM_SIZE_32 (1 << 14)
//     #define PERIPH_SIZE_32 ( 1 << 12)

//     #define DMA_MEM_TO_PERIPHERAL ( 1 << 6)


//     RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
//     DMA1_Stream5->CR = 0;
//     while(DMA1_Stream5->CR & DMA_SxCR_EN);

//     DMA1_Stream5->CR |= DMA_CHANNEL_3;
//     DMA1_Stream5->CR |= DMA_SxCR_CIRC;
//     DMA1_Stream5->CR |= DMA_SxCR_MINC;
//     DMA1_Stream5->CR |= MEM_SIZE_32;
//     DMA1_Stream5->CR |= PERIPH_SIZE_32;
//     DMA1_Stream5->CR |= DMA_MEM_TO_PERIPHERAL;

//     DMA1_Stream5->CR |= DMA_SxCR_TCIE;

//     DMA1_Stream5->NDTR = 5;
//     DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
//     DMA1_Stream5->M0AR = (uint32_t)(&intervals);  

//     NVIC_EnableIRQ(DMA1_Stream5_IRQn);
//     startDMA();

//     while(1){

//     }

// }


// void TIM2_IRQHandler(void) {
//     if (TIM2->SR & TIM_SR_UIF) {
//         TIM2->SR &=~ TIM_SR_UIF;
//         printf("UIF  %d\n",TIM2->CCR1);
    
//     }
//     else if(TIM2->SR & TIM_SR_CC1IF){
//         TIM2->SR &=~ TIM_SR_CC1IF;
//         printf("CC1IF %d\n",TIM2->CCR1);
//     }
// }

// void DMA1_Stream5_IRQHandler(void){
//     if(DMA1->HISR & DMA_HISR_TCIF5){
//         DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
//         printf("DMA DONE %d\n",TIM2->CCR1);
//     }
    
// }


// działający wzrost interwału (rośnie w timer update event a nie w compare event ponieważ jeżeli będzie rosnąć nigdy nie dojdzie do przerwania fali.
// void TIM2_IRQHandler(void) {
//     if (TIM2->SR & TIM_SR_UIF) {
//         TIM2->SR &=~ TIM_SR_UIF;
//         TIM2->CCR1 = intervals[counter];
//         ++counter;
//         if(counter == 8) counter = 0;
//         printf("1\n");
//     }
//     else if(TIM2->SR & TIM_SR_CC1IF){
//         TIM2->SR &=~ TIM_SR_CC1IF;
        
//          printf("%d\n",TIM2->CCR1);
//     }
// }

int __io_putchar(int ch){
    tx_send(ch);
    return ch;
}