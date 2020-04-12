#ifndef __MISC_H__
#define __MISC_H__
#include <stm32f10x.h>
#include <stddef.h>

void DMA1_Channel1_IRQHandler(void);
void dmaInit(uint32_t *src, uint32_t *dst, int size);
//void dmaInit(void);
void *memcpy32(void *dst, void const *src, size_t len);

#endif // __MISC_H__
