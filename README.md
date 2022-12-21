# Neopixels-bare-metal-STM32
Bare-metal implementation of WS2812B LED strip for STM32F446RE

## Timing data from documentation:
### WS2812:
- **Frequency: 800kHz**
- **Period: 1250ns** (1.25us)
- **Logic 0: 350ns HIGH -> 800ns LOW** (0.35/0.8us)
- **Logic 1: 700ns HIGH -> 600ns LOW** (0.7/0.6us)
- **Tolerance: +/- 150ns**
- **Reset: >50us**

### WS2812b:
- **Frequency: 800kHz**
- **Period: 1250ns** (1.25us)
- **Logic 0: 400ns HIGH -> 850ns LOW** (0.4/0.85us)
- **Logic 1: 800ns HIGH -> 450ns LOW** (0.8/0.45us)
- **Tolerance: +/- 150ns**  
- **Reset: >50us**

- *1000ns = 1us*
- *WS2812 has 6 legs, WS2812b has 4 legs.*
- *Total tolerance is +/- 600ns but the description seems vague because that's a lot.*
- *Tolerance of +/- 150ns gives us a small fliexibility to configure our timer.*
- *Reset signal ends sequence. After reset LED is counting again from first LED.*
- *Each LED requires 24bit data, it means each LED takes 30ms to update its color.*

### Timer configuration:
Setting timer to period of 1ns is not possible as it requires 1GHz clock. Below are some possible variants:

**Frequency / period**
- **20MHz / 50ns**
- 18MHz / 55.55ns
- 15MHz / 66.66ns
- 10MHz / 100ns