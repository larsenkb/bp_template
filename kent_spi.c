// with SPI_Direction_2Lines_FullDuplex
// I'm xmitting 5 bytes. There is gaps in the xmission...
// 2 bytes (no gap), then a gap, then a byte, then a gap, then two more bytes (no gap)
// with SPI_Direction_1Line_Tx
// there are not gaps

#include "kent_spi.h"
#include <stdio.h>


static bool wsDmaInProgress;

void spiDmaInit(void)
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
//	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; //= SPI_BaudRatePrescaler_128;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	///SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStruct);

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) ;

	wsDmaInProgress = false;
}


void spiDmaTest(uint8_t *buff, uint16_t size)
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
//	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStruct);

//    SPI_GPIO->BRR = SPI_PIN_MOSI;

	/* Enable the DMA channel */
	SPI_Cmd(SPI1, ENABLE);
	DMA_Cmd(DMA1_Channel3, ENABLE); /* Enable the DMA SPI TX Stream */

	/* Enable the SPI Rx/Tx DMA request */
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	wsDmaInProgress = true;

}

