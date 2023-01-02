# Neopixels bare metal for STM32
**Bare-metal implementation of WS2812 LED strip for STM32F446RE with PWM and DMA (Direct Memory Access).** During research I found many similar materials about working with WS2812 LED but all of them were for STM32 HAL without explaining the bare-metal concepts (some tutorials have in the title "bare-metal" but eventually misleads potential readers). Doing it from scratch was quite challenging. I guess I'm the first one to publish such thing without unnecessary complexity. It's in progress so I will appreciate any feedback.

## Timing constrains from documentation:

|                  | WS2812                  | WS2812b                 |
|-----------------:|:-----------------------:|:-----------------------:|
| **Frequency**    | 800kHz                  | 800kHz                  |
| **Period**       | 1250ns                  | 1250ns                  |
| **Logic 0**      | 350ns HIGH -> 800ns LOW | 400ns HIGH -> 850ns LOW |
| **Logic 1**      | 700ns HIGH -> 600ns LOW | 800ns HIGH -> 450ns LOW |
| **Duty cycle 0** | 28%                     | 32%                     |
| **Duty cycle 1** | 56%                     | 64%                     |
| **Tolerance**    | +/- 150ns (12%)         | +/- 150ns (12%)         |
| **Reset**        | > 50us                  | > 50us                  |

**Additional notes**
- *1000ns = 1us*
- *To distinguish what kind of LED strip you have: WS2812 has 6 legs, WS2812b has 4 legs.*
- *Sending order of colors is GRB.*
- *Total tolerance is +/- 600ns but the description seems vague because that's almost half of total period.*
- *Tolerance of +/- 150ns gives us a small fliexibility to configure our timer.*
- *Reset signal ends sequence. After reset LED is counting again from first LED. It won't clear the LED color.*
- *Each LED requires 24bit data, it means each LED takes 30us to update its color.*

## My Board settings:
### STM32F446RE clocks:
- Clocked at 180 MHz (full speed)
- APB1: 45MHz
- APB2: 90 MHz
- I'm using Timer 2 Channel 1 so it means it has 90MHz without prescaling. I'm using the prescaler of 2.

*Could my code be more flexible? Probably yes but it would introduce an enormous amount of complexity to configure correct timer and dma channels. I was focused on making something that works and someone who is seeing this can follow along.*

### Connection:
I've tested the code with 8 LED strip and 1 meter, 74 LED strip. STM32 operates in 3.3V logic so even small noise can cause visible glitches. Things to improve connection quality:
- Add capacitor between power and ground lines.
- Logic level converter for data pin. 
- External power supply (required for LED strips).

### Compile and upload:
1. In makefile add directory to CMSIS drivers.
2. Add directory to ST-Link
3. Type `make compile upload`

## Timer configuration:
**We are using PWM mode to pass the 0 and 1 as its pulse period.**
- Setting timer to period of 1ns is not possible as it requires 1GHz clock. Below are some possible variants. It depends how the clocks are set, we can use prescaler to get a desired frequency. 
- Ideally we should (but don't have to) set it to intigers to avoid approximation error and we can't set prescaler to a fractional number.
- Ticks is number that should be set in Timer ARR register (50 * 25 = 1250). Ticks 0 and 1 are the number for comparison in PWM mode.
- *Ticks for 0 and 1 is measured for WS2812 (350/700)
- **Ticks for 0 and 1 is measured for WS2812b (400/800)

*The table below is just an overview. You can set your own timer settings based on your clock configuration. 20MHz and 40MHz look like the best candidates without fractional numbers but other values can be fine as long as they fit into timing constrains.*

| Frequency | Period    | Ticks for 800 kHz | Ticks 0* | Ticks 1* | Ticks 0** | Ticks 1** | Prescaler: 90MHz          | 80MHz |
|:---------:|:---------:|:-----------------:|:--------:|:--------:|:--------:|:----------:|:-------------------------:|:-----:|
| 90MHz     | ~11.11ns  | ~112.5            | ~31.5    | ~63      | ~36      | ~72        | 1                         | -     |
| 80MHz     | 12.5ns    | 100               | 28       | 56       | 32       | 64         | -                         | 1     |
| 45MHz     | ~22.22ns  | ~56.25            | ~15.75   | ~31.5    | ~18      | ~36        | 2                         | -     |
| 40MHz     | 25ns      | 50                | 14       | 28       | 16       | 32         | -                         | 2     |
| 30MHz     | ~33.33ns  | ~37.5             | ~10.5    | ~21      | ~12      | ~24        | 3                         | -     |
| 20MHz     | 50ns      | 25                | 7        | 14       | 8        | 16         | -                         | 4     |
| 18MHz     | ~55.55ns  | ~22.5             | ~6.3     | ~12.6    | ~7.2     | ~14.4      | 5                         | -     |
| 16MHz     | 62.5ns    | 20                | 5.6      | 11.2     | 6.4      | 12.8       | -                         | 5     |
| 15MHz     | ~66.66ns  | ~18.75            | ~5.25    | ~10.5    | ~6       | ~12        | 6                         | -     |
| 10MHz     | 100ns     | ~12.5             | 3.5      | 7        | 4        | 8          | 9                         | 8     |

## DMA configuration:
*I still didn't figured which method would be the most efficient. In this code I'm not using the DMA circular mode.*
- Timer `CCDS` bit in `CR2` register changes the DMA requests from channel capture/compare event to timer update (when it reaches `ARR`). It's important because using `CC1E` to set thing can stack values when we compare shorter and then longer period. We want our pwm to send each bit with a single wave period.
- The pulse length changes with next value in queue when wave reaches its period. For each LED it does the change 24 times. After that it stops DMA requests, the flag to wait for next period is set and then it starts another signal.
- Reset signal is just a turn-off for 50us. You can use delay but it blocks the execution of rest things. It's better to temporaily change timer settings and return them back when another sequence is ready.

## Possible improvements:
**It's an ongoing project. I learned everything on my own and a lot of unanswered things still bother me:**
- *There is some serious issue when switching between the end of DMA packet transmission and reset signal which changes the timer settings. Previously even a small change in code could break this routine. Removing the systick interrupt solved the issue but probably it's not the case. I use the state guards to make it synchronous. How to make it truly a non-blocking routine?*
- *Furthermore, tutorials on the internet use PWM compare event to update the capture-compare register for next bit, then disabling DMA so it won't pass several values at once. I'm using the update event which should gently switch the states. For capture-compare event should I enable preload? (OC1PE)*
- *Does it make sense to define interrupts for both timer and DMA?*
- *How to not store DMA buffer as uint32_t to reduce its size (no, changing sizes in DMA won't work).*
- *Trying to debug PWM signals without an oscilloscope is not possible.*

## List of structs:
- `Led`: stores the rgb values.
- `LedStrip`: stores the pointer to led array, its size and strip state.

## List of functions:
- `timer_init()`: Configure the timer
- `dma_init()`: Configure the DMA.
- `setRGB(r, g, b)`: Set pixel color as RGB.
- `setHSB(h, s, b)`: Set pixel color as HSB and convert it to RGB. It's slower than setting directly as RGB. HSB is in range of 360, 100, 100.
- `createStrip(led_array, size)`: Create the strip struct.
- `updateStrip(strip)`: Update the strip.
- `clearStrip(strip)`: Clear the colors of led array. It doesn't update the strip automatically.
