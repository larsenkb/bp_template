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
#include "twi.h"
#include "bmp280.h"


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

//******************************************************************
// I2C
//******************************************************************
void I2C1_init(void)
{
    I2C_InitTypeDef  I2C_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Configure I2C_EE pins: SCL and SDA */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x38;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;

    /* I2C Peripheral Enable */
    I2C_Cmd(I2C1, ENABLE);
    /* Apply I2C configuration after enabling it */
    I2C_Init(I2C1, &I2C_InitStructure);
}


int main(void)
{
	register int i;
	uint32_t val1, val2;
	simple_float temp;
	char buff[80];
	long temperature, ti, tf;
	unsigned long pressure, pi, pf;
	
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
	sysTickDelay(100);
	
    // Initialize USART
    usart_init();
//    uart1_puts("USART1 initialized.\n\r");
	  write(1, "My name is Kent Larsen\n", 23);

	setup_delay_timer(TIM2);
	delay_us(TIM2, 100);

	I2C1_init();

#if 1
	// I2C scanner
//	twiEnable();
	printf("Scanning I2C bus...\n");
	twiScanPretty();
#endif

#if 1
	int id = BMP280_Init();
	id = id;
#endif
#if 1
	printf("ID:     %2x\n", id); sysTickDelay(2);
	sprintf(buff, "dig_T1: %6d\r\n", BMP280_Get(0)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_T2: %6d\r\n", BMP280_Get(1)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_T3: %6d\r\n", BMP280_Get(2)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P1: %6d\r\n", BMP280_Get(3)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P2: %6d\r\n", BMP280_Get(4)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P3: %6d\r\n", BMP280_Get(5)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P4: %6d\r\n", BMP280_Get(6)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P5: %6d\r\n", BMP280_Get(7)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P6: %6d\r\n", BMP280_Get(8)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P7: %6d\r\n", BMP280_Get(9)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P8: %6d\r\n", BMP280_Get(10)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "dig_P9: %6d\r\n", BMP280_Get(11)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "sizeof(int): %d\r\n", sizeof(int)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "sizeof(long): %d\r\n", sizeof(long)); uart1_puts(buff); sysTickDelay(2);
	sprintf(buff, "sizeof(long long): %d\r\n", sizeof(long long)); uart1_puts(buff); sysTickDelay(2);
#endif

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

	sysTickDelay(5);
	ds18b20_init(ONEWIRE_PORT, ONEWIRE_PIN, TIM2);

	while (1) {
//		sysTickDelay(5);
//		ds18b20_init(ONEWIRE_PORT, ONEWIRE_PIN, TIM2);
#if 1
		if (1) { //(FLAG_USART == 1) {
			bmp280Convert(&temperature, &pressure);
			ti = temperature/100;
			tf = temperature - ti*100;
			pi = pressure/256;
			pf = pressure - pi*256;
			pf *= 1000;
			pf /= 256;
			pf += 5;
			pf /= 10;
			//pressure /= 1013.25;

			sprintf(buff, "%d.%02d'C\n", (int)ti, (int)tf);
			uart1_puts(buff); //oledWriteString(0, 3, buff, FONT_NORMAL);
//			sprintf(buff, "%d.%02d Pa\n", (int)pi, (int)pf);
//			uart1_puts(buff); //oledWriteString(0, 5, buff, FONT_NORMAL);
//			sprintf(buff, "Temperature: %d.%02d, Pressure: %d.%02d\n", (int)ti, (int)tf, (int)pi, (int)pf);

			//USARTSendDMA(buff);
//    		uart1_puts(buff);
//			FLAG_USART = 0;
    	}
#endif
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



