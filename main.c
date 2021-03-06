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
#include "utils.h"
#include "ds18b20.h"


#define ARRAYSIZE 800
#define ONEWIRE_PIN		GPIO_Pin_1		// PA1
#define ONEWIRE_PORT	GPIOA			// PA1
#define PWR_PIN			GPIO_Pin_0		// PA0
#define PWR_PIN_PORT	GPIOA			// PA0

//volatile uint32_t status = 0;
uint32_t source[ARRAYSIZE];
uint32_t destination[ARRAYSIZE];

//******************************************************************
// External Power On
//******************************************************************
void Ext_Pwr_On(void)
{
	GPIO_WriteBit(PWR_PIN_PORT, PWR_PIN, 0);	// enb pwr to ext devices
}

//******************************************************************
// External Power Off
//******************************************************************
void Ext_Pwr_Off(void)
{
	GPIO_WriteBit(PWR_PIN_PORT, PWR_PIN, 1);	// enb pwr to ext devices
}

//******************************************************************
// External Power Initialize
//******************************************************************
void Ext_Pwr_Init(void)
{
//    I2C_InitTypeDef  I2C_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure the PWR pin */
	GPIO_InitStructure.GPIO_Pin = PWR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(PWR_PIN_PORT, &GPIO_InitStructure);

	Ext_Pwr_Off();

#if 0
	/* Configure the OneWire pin */
	GPIO_InitStructure.GPIO_Pin = ONEWIRE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, ONEWIRE_PIN); // Set PB8 to Low level ("0")
#endif
}


int main(void)
{
	register int i;
	uint32_t val1, val2;
	simple_float temp;
	char buff[80];
	
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

	// Power control to external peripherals
	Ext_Pwr_Init();
	Ext_Pwr_On();	// enable power to external peripherals
	
    // Initialize USART
    usart_init();
//    uart1_puts("USART1 initialized.\n\r");
//	  write(1, "My name is Kent Larsen\n", 23);

	setup_delay_timer(TIM2);
	delay_us(TIM2, 100);

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
//	dmaInit(source, destination, ARRAYSIZE);

	//LED on before transfer
	ledOn();

	//Enable DMA1 Channel transfer
//	DMA_Cmd(DMA1_Channel1, ENABLE);

	//wait for DMA transfer to be finished
	// Now go into stop mode, wake up on interrupt
//	asm("    wfi");
	
	val2 = SysTick->VAL;	
	printf("dma xfer dt: %ld\n", (val1-val2)/72);
	
	ledOff();
	ledOn();
	
//	memcpy32(destination, source, sizeof(destination));
	
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
		sysTickDelay(5);
		ds18b20_init(ONEWIRE_PORT, ONEWIRE_PIN, TIM2);
		do {
			temp = ds18b20_get_temperature_simple();
		} while (!temp.is_valid);
		sprintf(buff, "%d.%03d'C\r\n", temp.integer, temp.fractional);
		uart1_puts(buff);
		
		ledOn();
		sysTickDelay(1000);
		ledOff();
		sysTickDelay(1000);
	}
}



