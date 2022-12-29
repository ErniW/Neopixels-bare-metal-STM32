#include "./STM32F446RE/stm32f4xx.h"
#include "pll.h"
#include "sysTick.h"

#include "neopixels.h"
#include "serial.h"

#include <stdio.h>
#include <stdbool.h>

int main(){

    clockSpeed_PLL();  
    SysTick_Init();
    tx_init();

    timer_init();
    dma_init();

    Led strip[8];

    int value = 0;
    long prevTime = 0;

    while(1){
        long time = getMillis();

        if(time - prevTime > 5){

            for(int i = 0; i < 8; i++){
                strip[i] = setColor(value,value,value);
            }

            send(strip, 8);         
            value++;
            value = value % 255;
            prevTime = time;
        }
    }

}