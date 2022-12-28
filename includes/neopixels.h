#pragma once

typedef struct {
    int r;
    int g;
    int b;
} Led;

void timer_init();
void dma_init();

Led setColor(int r, int g, int b);

void send(Led *strip, int length);
void stop();