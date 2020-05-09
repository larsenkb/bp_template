/*--------------------------------------------------------------------
  The WS2812B library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  It is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  See <http://www.gnu.org/licenses/>.
  
  I've adapted to stm32f103 non-arduino. Kent B. Larsen 8May2020

  --------------------------------------------------------------------*/

#ifndef __WS2812B_H__
#define __WS2812B_H__

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_spi.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string.h>


// Using SPI1
#define SPI_GPIO      GPIOA
#define SPI_PIN_MOSI  GPIO_Pin_7
#define SPI_PIN_MISO  GPIO_Pin_6
#define SPI_PIN_SCK   GPIO_Pin_5


extern int wsInit(int nleds, int rgbw, int dbl);
extern int getNumPixels(void);

extern void wsShow(void);

extern void wsSetPixelColor_rgb(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
extern void wsSetPixelColor_rgbw(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

extern void wsSetPixelColor_c(uint16_t n, uint32_t c);
extern uint32_t wsColor_rgb(uint8_t r, uint8_t g, uint8_t b);
extern uint32_t wsColor_rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

extern void wsClear(void);
extern void wsColorWipe(uint32_t c, uint8_t dly);
extern void wsChaser(uint32_t c);
extern void wsTail(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4);


#endif // __WS2812B_H__
