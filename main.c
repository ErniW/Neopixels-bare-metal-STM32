#include "./STM32F446RE/stm32f4xx.h"
#include "pll.h"
#include "sysTick.h"

#include "neopixels.h"
#include "serial.h"

 #include <stdio.h>
#include <stdbool.h>

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
    // strip.isResetting = 0;
    // strip.isDone = 0;

    int value = 0;
    long prevTime = 0;

    while(1){
        long time = getMillis();

           if(time - prevTime > 20){
            
            for(int i = 0; i < strip.size; i++){
                strip.led[i] = setColor(value,0,0);
            }
           
            send(&strip);

            value++;
            value = value % 255;
            prevTime = time;

         }
    }

}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {

        TIM2->SR &=~ TIM_SR_UIF;
        
        if(strip.state == STATE_PACKET_DONE){
            strip.state = STATE_BUSY;
        }
        else if(strip.state == STATE_RESET_START){
            TIM2->ARR = WS2812_RESET_TICKS;
            strip.state = STATE_RESETTING;
        }
        else if(strip.state == STATE_RESETTING){
            TIM2->ARR = WS2812_FREQUENCY_800KHZ_TICKS;
            strip.state = STATE_IDLE;
            TIM2->CR1 &=~ TIM_CR1_CEN;
        }

    }
    if(TIM2->SR & TIM_SR_CC1IF){
        TIM2->SR &=~ TIM_SR_CC1IF; 
    }
}

void DMA1_Stream5_IRQHandler(void){
    if(DMA1->HISR & DMA_HISR_TCIF5){
        DMA1->HIFCR |= DMA_HIFCR_CTCIF5;        
        TIM2->CCR1 = 0;
        strip.state = STATE_PACKET_DONE;
    }
}

// int __io_putchar(int ch){
//     tx_send(ch);
//     return ch;
// }