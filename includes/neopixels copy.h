#pragma once

typedef struct {
    short g;
    short r;
    short b;
} Led;

void timer_init();
void dma_init(int vals, int peripheal, int size);

Led setColor(short r, short g, short b);

void send(Led* strip, int length);
void stop();