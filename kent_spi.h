#ifndef __KENT_SPI_H__
#define __KENT_SPI_H__

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_spi.h>
#include <stdbool.h>

// Using SPI1
#define SPI_GPIO      GPIOA
#define SPI_PIN_MOSI  GPIO_Pin_7
#define SPI_PIN_MISO  GPIO_Pin_6
#define SPI_PIN_SCK   GPIO_Pin_5


void spiDmaInit(void);
void spiDmaTest(uint8_t *buff, uint16_t size);

#endif // __KENT_SPI_H__
