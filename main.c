#include "./STM32F446RE/stm32f4xx.h"
#include "pll.h"
#include "sysTick.h"

#include "neopixels.h"
#include <math.h>

#define LEDS_AMOUNT 74

Led leds[LEDS_AMOUNT];
LedStrip strip;

int main(){

    clockSpeed_PLL();  
    // SysTick_Init();
    timer_init();
    dma_init();

    // SCB->CPACR |= ((3 << 10*2)|(3 << 11*2));

    strip = createStrip(leds, LEDS_AMOUNT);

    // long prevTime = 0;
    float movement = 0;

    while(1){


        // for(volatile long i = 0; i < 1000000; i++){
        //     // __ASM("NOP");
        // }

        // long time = getMillis();

        // if(time - prevTime > 50){
            movement  += 0.05;
            for(int i = 0; i < strip.size; i++){
                float sine = (sin((movement + ((i+1)*0.1))) + 1) / 2;
                sine *= 360;
                // uint8_t val = (uint8_t)sine;
                // if(val == 0) val += 10;
                strip.led[i] = setHSB(sine, 100,10);
                // printf()
            }
            // strip.led[strip.size-1] = setRGB(0,0,0);
            updateStrip(&strip);
            //movement  += 0.01;
            // prevTime = time;
        // }
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
            // TIM2->CR1 &=~ TIM_CR1_CEN;
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