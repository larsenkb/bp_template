#include <stm32f10x.h>
#include "misc.h"


void DMA1_Channel1_IRQHandler(void)
{
	//Test on DMA1 Channel1 Transfer Complete interrupt
	if (DMA_GetITStatus(DMA1_IT_TC1)) {
		//Clear DMA1 Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits
		DMA_ClearITPendingBit(DMA1_IT_GL1);
	}
}

void dmaInit(uint32_t *src, uint32_t *dst, int size)
{
	DMA_InitTypeDef  dcb;

	//enable DMA1 clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	//reset DMA1 channe1 to default values;
	DMA_DeInit(DMA1_Channel1);

	//channel will be used for memory to memory transfer
	dcb.DMA_M2M = DMA_M2M_Enable;

	//setting normal mode (non circular)
	dcb.DMA_Mode = DMA_Mode_Normal;

	//medium priority
	dcb.DMA_Priority = DMA_Priority_VeryHigh;

	//source and destination data size word=32bit
	dcb.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	dcb.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;

	//automatic memory increment enable. Destination and source
	dcb.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dcb.DMA_PeripheralInc = DMA_PeripheralInc_Enable;

	//Location assigned to peripheral register will be source
	dcb.DMA_DIR = DMA_DIR_PeripheralSRC;

	//chunk of data to be transfered
	dcb.DMA_BufferSize = size;

	//source and destination start addresses
	dcb.DMA_PeripheralBaseAddr = (uint32_t)src;
	dcb.DMA_MemoryBaseAddr = (uint32_t)dst;


	//send values to DMA registers
	DMA_Init(DMA1_Channel1, &dcb);

	// Enable DMA1 Channel Transfer Complete interrupt
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

	NVIC_InitTypeDef nvcb;

	//Enable DMA1 channel IRQ Channel */
	nvcb.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	nvcb.NVIC_IRQChannelPreemptionPriority = 0;
	nvcb.NVIC_IRQChannelSubPriority = 0;
	nvcb.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvcb);
}

void *memcpy32(void *dst, void const *src, size_t len)
{
	register uint32_t *ldst = (uint32_t *)dst;
	register uint32_t *lsrc = (uint32_t *)src;
	register size_t tlen = len;
	
	while (tlen >= 4) {
		*ldst++ = *lsrc++;
		tlen -= 4;
	}
	return dst;
}

