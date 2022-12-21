# Neopixels-bare-metal-STM32
Bare-metal implementation of WS2812B LED strip for STM32F446RE

## Timing data from documentation:
### WS2812:
- **Frequency: 800kHz**
- **Period: 1250ns** (1.25us)
- **Logic 0: 350ns HIGH -> 800ns LOW** (0.35/0.8us, 28/64%)
- **Logic 1: 700ns HIGH -> 600ns LOW** (0.7/0.6us, 56/48%)
- **Tolerance: +/- 150ns** (12%)
- **Reset: >50us**

### WS2812b:
- **Frequency: 800kHz**
- **Period: 1250ns** (1.25us)
- **Logic 0: 400ns HIGH -> 850ns LOW** (0.4/0.85us, 32/68%)
- **Logic 1: 800ns HIGH -> 450ns LOW** (0.8/0.45us, 64/36%)
- **Tolerance: +/- 150ns** (12%)
- **Reset: >50us**

**Additional notes**
- *1000ns = 1us*
- *WS2812 has 6 legs, WS2812b has 4 legs.*
- *Total tolerance is +/- 600ns but the description seems vague because that's almost half of total period.*
- *Tolerance of +/- 150ns gives us a small fliexibility to configure our timer.*
- *Reset signal ends sequence. After reset LED is counting again from first LED.*
- *Each LED requires 24bit data, it means each LED takes 30us to update its color.*

## Timer configuration:
- Setting timer to period of 1ns is not possible as it requires 1GHz clock. Below are some possible variants. It depends how the clocks are set, we can use prescaler to get a desired frequency. 
- Ideally we should set it to intigers to avoid approximation error and we can't set prescaler to a fractional number.
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
- It should set the timer CCR register when counter reloads the value and not when Capture/compare overflow event occurs. It will control the pulse length of PWM wave which eventually corresponds to logical 0 and 1. Using DMA is important as no other calculations should be done on transmission.
- After sending data of each LED, DMA should stop transmission and the reset signal should be sent.

### List of functions:
- Configure the timer
- Configure the DMA.
- Set pixel color in RGB
- Set pixel color in HSB
- Update the strip.
- Send the reset signal.
