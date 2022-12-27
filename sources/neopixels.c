#include "neopixels.h"
#include "../STM32F446RE/stm32f446xx.h"

/*

    1. init timer
    2. init dma
    3. interrupt kiedy pwm kończy sygnał

*/

void timer_init(){

}

void dma_init(){

};

Led setColor(short r, short g, short b){
    Led color = {g, r, b};
    return color;
};

void send(Led* strip, int length){

}
