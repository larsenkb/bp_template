/*
 * main.c
 *
 *  Created on: 9Apr2020
 */
// Includes ------------------------------------------------------------------*/
#include <stm32f10x.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "led.h"
#include "systickdelay.h"
#include "usart1.h"
#include "misc.h"
#include "xprintf.h"


#define ARRAYSIZE 800

//volatile uint32_t status = 0;
uint32_t source[ARRAYSIZE];
uint32_t destination[ARRAYSIZE];


int main(void)
{
	register int i;
	uint32_t val1, val2;
	
	// system comes up running HSI at external crystal frequency
	// which is 8MHz
		
	// Set clock
	SetSysClockTo72();			// SystemCoreClock
	SystemCoreClockUpdate();	// this should always be called after the above
	
	// SysTick Init
	sysTickInit();

	//initialize led
	ledInit();
	ledOff();
	
    // Initialize USART
    usart_init();
//    uart1_puts("USART1 initialized.\n\r");
//	  write(1, "My name is Kent Larsen\n", 23);

#if 0
	while(1) {
		ledToggle();
		sysTickDelay(1000);
	}
#endif
	
	//initialize source and destination arrays
	for (i = 0; i < ARRAYSIZE; i++) {
		source[i] = i;
		destination[i] = 0;
	}
	
	val1 = SysTick->VAL;	
	// Initialize DMA channel
	dmaInit(source, destination, ARRAYSIZE);

	//LED on before transfer
	ledOn();

	//Enable DMA1 Channel transfer
	DMA_Cmd(DMA1_Channel1, ENABLE);

	//wait for DMA transfer to be finished
	// Now go into stop mode, wake up on interrupt
	asm("    wfi");
	
	val2 = SysTick->VAL;	
	printf("dma xfer dt: %ld\n", (val1-val2)/72);
	
	ledOff();
	ledOn();
	
	memcpy32(destination, source, sizeof(destination));
	
#if 0
	ledOff();
	ledOn();
	
	memcpy(destination, source, sizeof(destination));
	
	ledOff();
	ledOn();
	
	for (i = 0; i < ARRAYSIZE; i++) {
		destination[i] = source[i];
	}
#endif
	
	ledOff();

	while (1) {
		//interrupts does the job
	}
}



