 #include "./STM32F446RE/stm32f4xx.h"


/*
    1. ustawić timer na mikrosekundy
    2. Ogarnąć kod zmieniania okresu i częstotliwości
    3. Ogarnąć sygnał reset
    4. Bitshift bitów w timerze by przesyłać dane.

------------- Na tym etapie powinienem umieć zapalić diody

    5. Przepisać na klasę
    6. Konwersja RGB <-> HSB

-------------
    Timing:
    Wartość 0:
    0.4 us HIGH -> 0.85 us LOW +/- 150 ns

    Wartość 1:
    0.8 us HIGH -> 0.45 us LOW +/- 150 ns

    Reset:
    LOW > 50 us

    HIGH + LOW = 1.25 us +/- 600 ns

    1 us = 1000 ns

    total 1250ns

    freq 400khz or 400hz???
    
    dma ch3 stream5


    todo dma ma wysyłać tylko kiedy jest overflow event.
*/

#include "./STM32F446RE/stm32f446xx.h"
#include<stdio.h>
#include<stdint.h>
#include "pll.h"
#include "sysTick.h"
#include "neopixels.h"
#include "serial.h"
#include <string.h>
// #include <iostream>
#include <stdbool.h>

// #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define STRIP_PIN   (1 << 8)
#define LED_AMOUNT  8
//dla ws2812b
// #define H_0         400
// #define L_0         850

// #define H_1         800
// #define L_1         450
//dla ws2812
#define H_0         16
#define L_0         800


#define H_1      32
#define L_1         600

#define RESET       50000
#define NEOPIXEL_UPDATE_PERIOD_NS 1250

#define PA5_AF_MODE (1 << 11)
#define PA5_AF1     (1 << 20)

#define CH1_PWM_MODE (6 << 4)



uint32_t vals[] = {
    // H_1,  H_1, H_1,H_1,H_1,H_1,H_1,H_1,H_0,
    // H_0, H_1,H_1,H_1,H_1,H_1,H_1,H_1,
    // H_0, H_0,H_1,H_0,H_0,H_0,H_0,H_1, 
    H_0, H_0,H_1,H_1,H_1,H_0,H_0,H_0,  
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,

     
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
    H_0, H_0,H_1,H_1,H_1,H_0,H_0,H_0,  

     
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
    H_0, H_0,H_1,H_1,H_1,H_0,H_0,H_0,  
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
    H_0, H_0,H_1,H_1,H_1,H_0,H_0,H_0,  
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
    H_0, H_0,H_1,H_1,H_1,H_0,H_0,H_0,  
    H_0, H_0,H_0,H_0,H_0,H_0,H_0,H_0,
};

int main(){

    clockSpeed_PLL();  
     SysTick_Init();
     //delay_ms(1000);
    // rx_init();
    // tx_init();
     delay_ms(10);
    //set the pin to AF mode and enable the timer
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
     delay_ms(10);

    GPIOA->MODER |= PA5_AF_MODE;
    GPIOA->AFR[0] |= PA5_AF1;

    //set the timer to 10 MHz
    TIM2->PSC = 2-1;
    TIM2->ARR = 56;//NEOPIXEL_UPDATE_PERIOD_NS;
    TIM2->CNT = 0;
    //set the period to 1250ns according to documentation
   
    TIM2->CCMR1 |= CH1_PWM_MODE;

    TIM2->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC1IE | TIM_DIER_UDE | TIM_DIER_TDE;
    TIM2->CR1 |= TIM_CR1_URS;
    
    //  TIM2->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;

         TIM2->CCR1 = 0;
         
    
    // TIM2->CR1 |= TIM_CR1_CEN;
    
    // __enable_irq();
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
     
    DMA1_Stream5->CR = 0;
     
    while(DMA1_Stream5->CR & DMA_SxCR_EN);

    DMA1_Stream5->CR |= (3 << 25); //ch3

    //   DMA1_Stream5->CR |= DMA_SxCR_CIRC;
    DMA1_Stream5->CR |= DMA_SxCR_MINC;

    	/*Set Mem size :  32bit*/
	DMA1_Stream5->CR &=~(1U<<13);
	DMA1_Stream5->CR |=(1U<<14);

	/*Set Periph size :  32bit*/
	DMA1_Stream5->CR &=~(1U<<11);
	DMA1_Stream5->CR |=(1U<<12);

	/*Set Transfer direction : Mem to Periph */
	DMA1_Stream5->CR |=(1U<<6);
	DMA1_Stream5->CR &=~(1U<<7);

    	/*Set number of transfer*/
	DMA1_Stream5->NDTR = sizeof(vals)/sizeof(uint32_t);

	/*Set peripheral address*/
	DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
	/*Set memory address*/
	DMA1_Stream5->M0AR = (uint32_t)(&vals);
 
	/*Enable the DMA Stream*/
	uint8_t counter = 0;
      

    //  NVIC_EnableIRQ(TIM2_IRQn);
        TIM2->CCER |= TIM_CCER_CC1E; 
    TIM2->CR1 |= TIM_CR1_CEN;
 
    DMA1_Stream5->NDTR = sizeof(vals)/sizeof(uint32_t);

	/*Set peripheral address*/
	DMA1_Stream5->PAR =  (uint32_t)(&TIM2->CCR1);
	/*Set memory address*/
	DMA1_Stream5->M0AR = (uint32_t)(&vals);
     
         DMA1_Stream5->CR |=  DMA_SxCR_EN;
          delay_ms(1);
         TIM2->CCR1 = 0;
         DMA1_Stream5->CR &=~  DMA_SxCR_EN;
    
    while(1){
        //   TIM2->CCR1 = 0;
    
        


        // for(int i=0; i< 24; i++){
        //     // if(vals[i] = H_1) vals[i] = H_0;
        //     // else vals[i] = H_1;
        // }
        
        
        // delay_ms(1000);
        // TIM2->CCER &=~ TIM_CCER_CC1E;        
        // delay_ms(10);      
        // TIM2->CNT = 0; 
        // TIM2->CCER |= TIM_CCER_CC1E;    
        // DMA1_Stream5->CR |=  DMA_SxCR_EN;
        // DMA1_Stream5->CR |=  DMA_SxCR_EN;
     // DMA1_Stream5->CR |=  DMA_SxCR_EN;
    //  TIM2->CR1 |= TIM_CR1_CEN;
       //delay_ms(1000); 
    //   DMA1_Stream5->CR |=  DMA_SxCR_EN;

   // TIM2->CR1 |= TIM_CR1_CEN;
      
     // TIM2->CCER &=~ TIM_CCER_CC1E; 
      //TIM2->CR1 &=~ TIM_CR1_CEN;  
    //   TIM2->CNT = 0;

    //   DMA1_Stream5->CR &=~  DMA_SxCR_EN;
    //    RCC->APB1ENR &=~ RCC_APB1ENR_TIM2EN;
    }

}

 volatile int counter = 0;

void TIM2_IRQHandler(void) {
    
    if((TIM2->SR & TIM_SR_CC1IF)){
       // TIM2->SR &=~ TIM_SR_CC1IF;
        
        // if(counter == 23) TIM2->CR1 &=~ TIM_CR1_CEN;
        // counter++;
    }
}

int __io_putchar(int ch){
    tx_send(ch);
    return ch;
}