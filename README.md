# Bluepill low level programming
This is a project where I have been doing low level programming of the STM32F103 bluepill board. By that I mean using GCC and a Makefile and the code from ST for the STM32F103.

I'm using the ST-Link-v2 to do the flashing and debugging.

I can select the clock speed: 8 (HSE or HSI), 24MHz, 36MHz, 48MHz, 56MHz, and 72MHz.

SysTick is working. There are delay routines (delay by ticks).

I have USART1 working and I have USART connected to 'stdout'. So 'printf' and 'write' work (sort of).

I can do memory to memory DMA.

There is code to control the built in LED (PC13).

I am using [ChaN](http://elm-chan.org/) xprintf

I started to code base by taking some code from [embedds](https://embedds.com/using-direct-memory-access-dma-in-stm23-projects/) and modifying it for gcc and makefile.

## Planned additions
Some of the features I would like to get working in the near future are:
* Add 'stdin' support
* Add I2C support
* Add SPI support
