/*-----------------------------------------------------------------------------------------------
  Arduino library to control WS2812B RGB Led strips using the Arduino STM32 LibMaple core
  -----------------------------------------------------------------------------------------------
 
  Note. 
  This library has only been tested on the WS2812B LED. It may not work with the older WS2812 or
  other types of addressable RGB LED, becuase it relies on a division multiple of the 72Mhz clock 
  frequence on the STM32 SPI to generate the correct width T0H pulse, of 400ns +/- 150nS
  SPI DIV32 gives a pulse width of 444nS which is well within spec for the WS2812B but
  is probably too long for the WS2812 which needs a 350ns pulse for T0H
 
  This WS2811B library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  It is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  See   <http://www.gnu.org/licenses/>.

  I've adapted to stm32f103 non-arduino. Kent B. Larsen 8May2020

  -----------------------------------------------------------------------------------------------*/

#include "ws2812b.h"
#include "systickdelay.h"


#define NUM_LEDS		16

#define RGBW			0

#if RGBW == 1
#define BYTES_PER_PIXEL	12
#else
#define BYTES_PER_PIXEL	9
#endif

//static uint8_t *encoderLookup;
static uint8_t		pixels[2][NUM_LEDS * BYTES_PER_PIXEL + 1];
static int pIdx = 0;

//static bool		begun = false;         // true if begin() previously called
static uint16_t	numLEDs;       // Number of RGB LEDs in strip
static uint16_t	numBytes;      // Size of 'pixels' buffer

extern uint8_t encoderLookup[];

static bool wsDmaInProgress;


static void wsSpiStartXfer(uint8_t *buff, uint16_t size);
static void wsSpiInit(void);

void wsInit(void)
{
	wsSpiInit();
	pIdx = 0;
	wsDmaInProgress = false;
	numLEDs = NUM_LEDS;
	numBytes = NUM_LEDS * BYTES_PER_PIXEL;
//    begun = true;
//	pixels = doubleBuffer; // + 1;
}

int getNumPixels(void)
{
	return (int)numLEDs;
}

// Sends the current buffer to the leds
void wsShow(void) 
{
	extern void spiDmaTest(uint8_t *buf, uint16_t size);
	wsSpiStartXfer(pixels[pIdx], numBytes + 1);  // Start DMA xfer and return immediately.
  	memcpy(pixels[pIdx^1], pixels[pIdx], numBytes + 1);// copy first buffer to second buffer
	pIdx ^= 1;
}


void wsSetPixelColor_rgb(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
   uint8_t *bptr = &pixels[pIdx][n * BYTES_PER_PIXEL];
   uint8_t *tPtr = &encoderLookup[g * 3]; // + g*2 + g;// need to index 3 x g into the lookup
   
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;

   tPtr = &encoderLookup[r * 3]; // + r*2 + r;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;   
   
   tPtr = &encoderLookup[b * 3]; // + b*2 + b;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
}

void wsSetPixelColor_rgbw(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
   uint8_t *bptr = &pixels[pIdx][n * BYTES_PER_PIXEL];
   uint8_t *tPtr = &encoderLookup[g * 3]; // + g*2 + g;// need to index 3 x g into the lookup
   
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;

   tPtr = &encoderLookup[r * 3]; // + r*2 + r;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;   
   
   tPtr = &encoderLookup[b * 3]; // + b*2 + b;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   
   tPtr = &encoderLookup[w * 3]; // + w*2 + w;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
 }


// Packed format is always WRGB, regardless of LED strand color order.
void wsSetPixelColor_c(uint16_t n, uint32_t c)
{
	uint16_t r,g,b;
#if RGBW
	uint16_t w;
#endif
   
//	r = (uint16_t)((c >> 16) & 0xff),
//	g = (uint16_t)((c >>  8) & 0xff),
//	b = (uint16_t)(c & 0xff);		

	b = (uint16_t)(c & 0xff);	
	g = (uint16_t)((c >> 8) & 0xff);	
	r = (uint16_t)((c >> 16) & 0xff);	
#if RGBW
	w = (uint16_t)((c >> 24) & 0xff);	
#endif
	
   uint8_t *bptr = &pixels[pIdx][n * BYTES_PER_PIXEL];

   uint8_t *tPtr = &encoderLookup[g * 3]; // + g*2 + g;// need to index 3 x g into the lookup
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;

   tPtr = &encoderLookup[r * 3]; // + r*2 + r;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;   
   
   tPtr = &encoderLookup[b * 3]; // + b*2 + b;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;

#if RGBW
   tPtr = &encoderLookup[w * 3]; // + w*2 + w;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
   *bptr++ = *tPtr++;
#endif
}


// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t wsColor_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

#if 1
// Convert separate R,G,B,W into packed 32-bit WRGB color.
// Packed format is always WRGB, regardless of LED strand color order.
uint32_t wsColor_rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}
#endif

// Fill the LEDs one after the other with a color
void wsColorWipe(uint32_t c, uint8_t dly) 
{
  if (dly) {
    for (uint16_t i = 0; i < numLEDs; i++) {
      wsSetPixelColor_c(i, c);
      wsShow();
      sysTickDelay(dly);
    }
  } else {
    for (uint16_t i = 0; i < numLEDs; i++) {
      wsSetPixelColor_c(i, c);
    }
    wsShow();
    sysTickDelay(1);
  }
}

void wsChaser(uint32_t c) 
{
	static int idx = 0;
	uint16_t mask = numLEDs - 1;
	
	wsSetPixelColor_c(idx, 0);
//	sysTickDelay(2);
	idx = (idx + 1) & mask;
	wsSetPixelColor_c(idx, c);
}

void wsTail(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4)
{
	static int idx = 0;
	int tidx;
	uint16_t mask = numLEDs - 1;
	
	wsSetPixelColor_c(idx, c1);
	tidx = (idx - 1) & mask;
	wsSetPixelColor_c(tidx, c2);
	tidx = (tidx - 1) & mask;
	wsSetPixelColor_c(tidx, c3);
	tidx = (tidx - 1) & mask;
	wsSetPixelColor_c(tidx, c4);
//	sysTickDelay(2);
	idx = (idx + 1) & mask;
//	wsSetPixelColor_c(idx, c);
}

//
//	Sets the encoded pixel data to turn all the LEDs off.
//
void wsClear(void) 
{
	pIdx = 0;
	uint8_t *bptr = pixels[pIdx];// Note first byte in the buffer is a preamble and is always zero. hence the +1
	uint8_t *tPtr;

#if RGBW
	for (int i = 0; i < numLEDs * 4; i++) {
	   tPtr = (uint8_t *)encoderLookup;
   	   *bptr++ = *tPtr++;
	   *bptr++ = *tPtr++;
	   *bptr++ = *tPtr++;	
//	   *bptr++ = *tPtr++;	
	}
	*bptr++ = 0;	
#else
	for (int i = 0; i < numLEDs * 3; i++) {
	   tPtr = (uint8_t *)encoderLookup;
   	   *bptr++ = *tPtr++;
	   *bptr++ = *tPtr++;
	   *bptr++ = *tPtr++;	
	}
	*bptr++ = 0;	
#endif
}



static void wsSpiInit(void)
{
	// RCC Clock Setting
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

	// GPIO Setting
	// Set SCLK, MISO and MOSI pin of SPI peripheral to pull-down and map 'Alternate Function' to those pins.
	GPIO_InitTypeDef GPIO_InitStruct;
    // GPIO pins for MOSI, MISO, and SCK
    GPIO_InitStruct.GPIO_Pin = SPI_PIN_MOSI | SPI_PIN_MISO | SPI_PIN_SCK;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI_GPIO, &GPIO_InitStruct);

	// SPI Setting
	// Set SPI Direction as FullDuplex and SPI Prescaler with SPI_BaudRatePrescaler_2 which is the fastest one.
	SPI_InitTypeDef SPI_InitStruct;
	SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; //= SPI_BaudRatePrescaler_128;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;

	SPI_Init(SPI1, &SPI_InitStruct);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) ;

	wsDmaInProgress = false;
}


static void wsSpiStartXfer(uint8_t *buff, uint16_t size)
{
	if (wsDmaInProgress) {
		/* Waiting the end of Data transfer */
		while (DMA_GetFlagStatus(DMA1_FLAG_TC3) == RESET);
		DMA_Cmd(DMA1_Channel3, DISABLE);
		DMA_ClearFlag(/*DMA1_Channel3,*/ DMA1_FLAG_TC3);

		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET) ;
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET) ;
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
		SPI_Cmd(SPI1, DISABLE);

		SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_BSY);
		SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_TXE);
		wsDmaInProgress = false;
	}
	
	DMA_DeInit(DMA1_Channel3); //SPI1_TX_DMA_STREAM
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_BufferSize = (uint16_t)size;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR);
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)buff;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStruct);


	/* Enable the DMA channel */
	SPI_Cmd(SPI1, ENABLE);
	DMA_Cmd(DMA1_Channel3, ENABLE); /* Enable the DMA SPI TX Stream */

	/* Enable the SPI Rx/Tx DMA request */
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	wsDmaInProgress = true;

}

uint8_t encoderLookup[] =
                    { 0x92,0x49,0x24,0x92,0x49,0x26,0x92,0x49,0x34,0x92,0x49,0x36,0x92,0x49,0xA4,0x92,
                      0x49,0xA6,0x92,0x49,0xB4,0x92,0x49,0xB6,0x92,0x4D,0x24,0x92,0x4D,0x26,0x92,0x4D,
                      0x34,0x92,0x4D,0x36,0x92,0x4D,0xA4,0x92,0x4D,0xA6,0x92,0x4D,0xB4,0x92,0x4D,0xB6,
                      0x92,0x69,0x24,0x92,0x69,0x26,0x92,0x69,0x34,0x92,0x69,0x36,0x92,0x69,0xA4,0x92,
                      0x69,0xA6,0x92,0x69,0xB4,0x92,0x69,0xB6,0x92,0x6D,0x24,0x92,0x6D,0x26,0x92,0x6D,
                      0x34,0x92,0x6D,0x36,0x92,0x6D,0xA4,0x92,0x6D,0xA6,0x92,0x6D,0xB4,0x92,0x6D,0xB6,
                      0x93,0x49,0x24,0x93,0x49,0x26,0x93,0x49,0x34,0x93,0x49,0x36,0x93,0x49,0xA4,0x93,
                      0x49,0xA6,0x93,0x49,0xB4,0x93,0x49,0xB6,0x93,0x4D,0x24,0x93,0x4D,0x26,0x93,0x4D,
                      0x34,0x93,0x4D,0x36,0x93,0x4D,0xA4,0x93,0x4D,0xA6,0x93,0x4D,0xB4,0x93,0x4D,0xB6,
                      0x93,0x69,0x24,0x93,0x69,0x26,0x93,0x69,0x34,0x93,0x69,0x36,0x93,0x69,0xA4,0x93,
                      0x69,0xA6,0x93,0x69,0xB4,0x93,0x69,0xB6,0x93,0x6D,0x24,0x93,0x6D,0x26,0x93,0x6D,
                      0x34,0x93,0x6D,0x36,0x93,0x6D,0xA4,0x93,0x6D,0xA6,0x93,0x6D,0xB4,0x93,0x6D,0xB6,
                      0x9A,0x49,0x24,0x9A,0x49,0x26,0x9A,0x49,0x34,0x9A,0x49,0x36,0x9A,0x49,0xA4,0x9A,
                      0x49,0xA6,0x9A,0x49,0xB4,0x9A,0x49,0xB6,0x9A,0x4D,0x24,0x9A,0x4D,0x26,0x9A,0x4D,
                      0x34,0x9A,0x4D,0x36,0x9A,0x4D,0xA4,0x9A,0x4D,0xA6,0x9A,0x4D,0xB4,0x9A,0x4D,0xB6,
                      0x9A,0x69,0x24,0x9A,0x69,0x26,0x9A,0x69,0x34,0x9A,0x69,0x36,0x9A,0x69,0xA4,0x9A,
                      
                      0x69,0xA6,0x9A,0x69,0xB4,0x9A,0x69,0xB6,0x9A,0x6D,0x24,0x9A,0x6D,0x26,0x9A,0x6D,
                      0x34,0x9A,0x6D,0x36,0x9A,0x6D,0xA4,0x9A,0x6D,0xA6,0x9A,0x6D,0xB4,0x9A,0x6D,0xB6,
                      0x9B,0x49,0x24,0x9B,0x49,0x26,0x9B,0x49,0x34,0x9B,0x49,0x36,0x9B,0x49,0xA4,0x9B,
                      0x49,0xA6,0x9B,0x49,0xB4,0x9B,0x49,0xB6,0x9B,0x4D,0x24,0x9B,0x4D,0x26,0x9B,0x4D,
                      0x34,0x9B,0x4D,0x36,0x9B,0x4D,0xA4,0x9B,0x4D,0xA6,0x9B,0x4D,0xB4,0x9B,0x4D,0xB6,
                      0x9B,0x69,0x24,0x9B,0x69,0x26,0x9B,0x69,0x34,0x9B,0x69,0x36,0x9B,0x69,0xA4,0x9B,
                      0x69,0xA6,0x9B,0x69,0xB4,0x9B,0x69,0xB6,0x9B,0x6D,0x24,0x9B,0x6D,0x26,0x9B,0x6D,
                      0x34,0x9B,0x6D,0x36,0x9B,0x6D,0xA4,0x9B,0x6D,0xA6,0x9B,0x6D,0xB4,0x9B,0x6D,0xB6,
                      0xD2,0x49,0x24,0xD2,0x49,0x26,0xD2,0x49,0x34,0xD2,0x49,0x36,0xD2,0x49,0xA4,0xD2,
                      0x49,0xA6,0xD2,0x49,0xB4,0xD2,0x49,0xB6,0xD2,0x4D,0x24,0xD2,0x4D,0x26,0xD2,0x4D,
                      0x34,0xD2,0x4D,0x36,0xD2,0x4D,0xA4,0xD2,0x4D,0xA6,0xD2,0x4D,0xB4,0xD2,0x4D,0xB6,
                      0xD2,0x69,0x24,0xD2,0x69,0x26,0xD2,0x69,0x34,0xD2,0x69,0x36,0xD2,0x69,0xA4,0xD2,
                      0x69,0xA6,0xD2,0x69,0xB4,0xD2,0x69,0xB6,0xD2,0x6D,0x24,0xD2,0x6D,0x26,0xD2,0x6D,
                      0x34,0xD2,0x6D,0x36,0xD2,0x6D,0xA4,0xD2,0x6D,0xA6,0xD2,0x6D,0xB4,0xD2,0x6D,0xB6,
                      0xD3,0x49,0x24,0xD3,0x49,0x26,0xD3,0x49,0x34,0xD3,0x49,0x36,0xD3,0x49,0xA4,0xD3,
                      0x49,0xA6,0xD3,0x49,0xB4,0xD3,0x49,0xB6,0xD3,0x4D,0x24,0xD3,0x4D,0x26,0xD3,0x4D,
                      
                      0x34,0xD3,0x4D,0x36,0xD3,0x4D,0xA4,0xD3,0x4D,0xA6,0xD3,0x4D,0xB4,0xD3,0x4D,0xB6,
                      0xD3,0x69,0x24,0xD3,0x69,0x26,0xD3,0x69,0x34,0xD3,0x69,0x36,0xD3,0x69,0xA4,0xD3,
                      0x69,0xA6,0xD3,0x69,0xB4,0xD3,0x69,0xB6,0xD3,0x6D,0x24,0xD3,0x6D,0x26,0xD3,0x6D,
                      0x34,0xD3,0x6D,0x36,0xD3,0x6D,0xA4,0xD3,0x6D,0xA6,0xD3,0x6D,0xB4,0xD3,0x6D,0xB6,
                      0xDA,0x49,0x24,0xDA,0x49,0x26,0xDA,0x49,0x34,0xDA,0x49,0x36,0xDA,0x49,0xA4,0xDA,
                      0x49,0xA6,0xDA,0x49,0xB4,0xDA,0x49,0xB6,0xDA,0x4D,0x24,0xDA,0x4D,0x26,0xDA,0x4D,
                      0x34,0xDA,0x4D,0x36,0xDA,0x4D,0xA4,0xDA,0x4D,0xA6,0xDA,0x4D,0xB4,0xDA,0x4D,0xB6,
                      0xDA,0x69,0x24,0xDA,0x69,0x26,0xDA,0x69,0x34,0xDA,0x69,0x36,0xDA,0x69,0xA4,0xDA,
                      0x69,0xA6,0xDA,0x69,0xB4,0xDA,0x69,0xB6,0xDA,0x6D,0x24,0xDA,0x6D,0x26,0xDA,0x6D,
                      0x34,0xDA,0x6D,0x36,0xDA,0x6D,0xA4,0xDA,0x6D,0xA6,0xDA,0x6D,0xB4,0xDA,0x6D,0xB6,
                      0xDB,0x49,0x24,0xDB,0x49,0x26,0xDB,0x49,0x34,0xDB,0x49,0x36,0xDB,0x49,0xA4,0xDB,
                      0x49,0xA6,0xDB,0x49,0xB4,0xDB,0x49,0xB6,0xDB,0x4D,0x24,0xDB,0x4D,0x26,0xDB,0x4D,
                      0x34,0xDB,0x4D,0x36,0xDB,0x4D,0xA4,0xDB,0x4D,0xA6,0xDB,0x4D,0xB4,0xDB,0x4D,0xB6,
                      0xDB,0x69,0x24,0xDB,0x69,0x26,0xDB,0x69,0x34,0xDB,0x69,0x36,0xDB,0x69,0xA4,0xDB,
                      0x69,0xA6,0xDB,0x69,0xB4,0xDB,0x69,0xB6,0xDB,0x6D,0x24,0xDB,0x6D,0x26,0xDB,0x6D,
                      0x34,0xDB,0x6D,0x36,0xDB,0x6D,0xA4,0xDB,0x6D,0xA6,0xDB,0x6D,0xB4,0xDB,0x6D,0xB6
};


