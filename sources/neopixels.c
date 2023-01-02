#include "../STM32F446RE/stm32f446xx.h"
#include <math.h>
#include "sysTick.h"
#include "neopixels.h"

#define PA5_AF_MODE             (1 << 11)
#define PA5_AF1                 (1 << 20)
#define CH1_PWM_MODE_1          (6 << 4)

#define DMA_CHANNEL_3           (3 << 25)
#define DMA_MEM_TO_PERIPHERAL   (1 << 6)
#define MEM_SIZE_32             (1 << 14)
#define PERIPH_SIZE_32          (1 << 12)

void timer_init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    GPIOA->MODER |= PA5_AF_MODE;
    GPIOA->AFR[0] |= PA5_AF1;

    TIM2->PSC = TIMER_PRESCALER - 1;
    TIM2->ARR = WS2812_FREQ_800KHZ_TICKS;
    TIM2->CCMR1 = CH1_PWM_MODE_1;
    // TIM2->CCMR1 |= TIM_CCMR1_OC1PE;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->DIER |= TIM_DIER_UDE;
    TIM2->DIER |= TIM_DIER_CC1IE;
    TIM2->DIER |= TIM_DIER_CC1DE;
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

void updateStrip(LedStrip *strip){
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

Led setRGB(uint8_t r, uint8_t g, uint8_t b){
    Led color = {r, g, b};
    return color;
};

Led setHSB(int hue, uint8_t sat, uint8_t bright){
    double s = ((double)sat) / 100;
    double br = ((double)bright) / 100;

    double H = (double)hue;
    double S = s;
    double B = br;

    double P = 0;
    double Q = 0;
    double T = 0;

    if (H == 360){
        H = 0;
    }
    else {
        H /= 60;
    }

    double fract = H - floor(H);
    double r = 0;
    double g = 0;
    double b = 0;

    P = B * (1. - S);
    Q = B * (1. - S * fract);
    T = B * (1. - S * (1. - fract));

    if (H >= 0 && H < 1) {
        r = B; 
        g = T;
        b = P;
    }
    else if (H >= 1 && H < 2) {
        r = Q;
        g = B;
        b = P;
    }
    else if (H >= 2 && H < 3) {
        r = P;
        g = B;
        b = T;
    }
    else if (H >= 3 && H < 4) {
        r = P;
        g = Q;
        b = B;
    }
    else if (H >= 4 && H < 5) {
        r = T;
        g = P;
        b = B;
    }
    else if (H >= 5 && H < 6) {
        r = B;
        g = P;
        b = Q;
    }
    else {
        r = 0;
        g = 0;
        b = 0;
    }

    Led newColor = {
        (uint8_t)(r * 255), 
        (uint8_t)(g * 255),
        (uint8_t)(b * 255)
    };

    return newColor;
}
LedStrip createStrip(Led* led_array_ptr, uint16_t size){
    LedStrip newStrip = {led_array_ptr, size, STATE_IDLE};
    return newStrip;
};

void clearStrip(LedStrip* strip){
    for(int i=0; i<strip->size; ++i){
        strip->led[i] = setRGB(0,0,0);
    }
}