#pragma once
#include "../STM32F446RE/stm32f4xx.h"

#define TIMER_PRESCALER          2
#define WS2812_FREQ_800KHZ_TICKS 56
#define LOGIC_0_TICKS            16
#define LOGIC_1_TICKS            32
#define WS2812_RESET_TICKS       2250

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Led;

typedef struct {
    Led* led;
    uint16_t size;
    volatile uint8_t state;
} LedStrip;

enum{
    STATE_IDLE,
    STATE_BUSY,
    STATE_PACKET_DONE,
    STATE_RESET_START,
    STATE_RESETTING
};

void timer_init();
void dma_init();

LedStrip createStrip(Led* led_array_ptr, uint16_t size);
void updateStrip(LedStrip *strip);
void clearStrip(LedStrip *strip);

Led setHSB(int h, uint8_t s, uint8_t b);
Led setRGB(uint8_t r, uint8_t g, uint8_t b);
Led getHSB(Led *c);