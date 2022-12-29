#pragma once
#include "../STM32F446RE/stm32f4xx.h"
#include "../STM32F446RE/stm32f446xx.h"

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Led;

enum{
    STATE_IDLE,
    STATE_BUSY,
    STATE_RESETTING
};

void timer_init();
void dma_init();

Led setColor(uint8_t r, uint8_t g, uint8_t b);

void send(Led *strip, int length);
void stop();