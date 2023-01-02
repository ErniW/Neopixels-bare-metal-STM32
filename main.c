#include "./STM32F446RE/stm32f4xx.h"

#include <math.h>
#include "pll.h"
#include "sysTick.h"
#include "neopixels.h"

#define ENABLE_FPU ((3 << 20) | (3 << 22))

#define LEDS_AMOUNT 74

Led leds[LEDS_AMOUNT];
LedStrip strip;

int main(){

    clockSpeed_PLL();  
    // SysTick_Init();
    timer_init();
    dma_init();

    SCB->CPACR |= ENABLE_FPU;

    float movement = 0;

    strip = createStrip(leds, LEDS_AMOUNT);

    while(1){

        if(strip.state == STATE_IDLE){

            movement  += 0.05;

            for(int i = 0; i < strip.size; ++i){
                float sine = sinf(movement + (i+1) * 0.1 );
                sine = (sine + 1) / 2;
                sine *= 360;
                strip.led[i] = setHSB(sine, 100, 20);
            }

            updateStrip(&strip);
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
            TIM2->ARR = WS2812_FREQ_800KHZ_TICKS;
            strip.state = STATE_IDLE;
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