#include "./STM32F446RE/stm32f4xx.h"
#include "pll.h"
#include "sysTick.h"

#include "neopixels.h"
#include "serial.h"

 #include <stdio.h>
// #include <stdbool.h>

Led leds[8];
LedStrip strip;

int main(){

    clockSpeed_PLL();  
    SysTick_Init();
     tx_init();

    timer_init();
    dma_init();



    strip.led = leds;
    strip.size = 8;
    strip.state = STATE_IDLE;
    strip.isResetting = 0;
    strip.isDone = 0;

    int value = 0;
    long prevTime = 0;

           

    while(1){
        long time = getMillis();

        if(time - prevTime > 10){
             for(int i = 0; i < 8; i++){
                strip.led[i] = setColor(value,value,value);
            }

            send(&strip);
                
            // strip.state = STATE_IDLE;     
            value++;
            value = value % 255;
            prevTime = time;
        }
    }

}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &=~ TIM_SR_UIF;

        if(strip.isDone) {       
            TIM2->CCR1 = 0;
            strip.isDone = 0;
            // printf("PACKET DONE\n");
        }

        if(strip.isResetting){
            
            // printf("STOP RESETTING\n");
            strip.isResetting = 0;
            TIM2->ARR = WS2812_FREQUENCY_800KHZ_TICKS;
        }
    }
    else if(TIM2->SR & TIM_SR_CC1IF){
        TIM2->SR &=~ TIM_SR_CC1IF; 
    }
}
// extern volatile int counter;

void DMA1_Stream5_IRQHandler(void){
    if(DMA1->HISR & DMA_HISR_TCIF5){
        DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
        strip.isDone = 1;
        //counter ++;
       
        if(strip.isResetting){
            //   printf("START RESET\n");
              TIM2->ARR = WS2812_RESET_TICKS;
        }
    }
}

int __io_putchar(int ch){
    tx_send(ch);
    return ch;
}