#pragma once
#include "../STM32F446RE/stm32f4xx.h"

#define TIMER_PRESCALER 2
#define WS2812_FREQUENCY_800KHZ_TICKS 56
#define LOGIC_0_TICKS 16
#define LOGIC_1_TICKS 32
#define WS2812_RESET_TICKS 2250

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Led;

typedef struct {
    Led* led;
    uint16_t size;
    uint8_t state;
    volatile int isResetting;
    volatile int isDone;
} LedStrip;

enum{
    STATE_IDLE,
    STATE_BUSY,
    STATE_PACKET_DONE,
    STATE_RESETTING
};

void timer_init();
void dma_init();

Led setColor(uint8_t r, uint8_t g, uint8_t b);

void send(LedStrip *strip);
void stop();