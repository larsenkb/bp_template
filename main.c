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
#include "oled96.h"
#include "ds18b20.h"
#include "twi.h"
#include "bmp280.h"
#include "kent_spi.h"
#include "ws2812b.h"

#include "platform.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"


#define ARRAYSIZE 800
#define ONEWIRE_PIN		GPIO_Pin_10		// PB10
#define ONEWIRE_PORT	GPIOB			// PB10
#define PWR_PIN			GPIO_Pin_0		// PA0
#define PWR_PIN_PORT	GPIOA			// PA0

#define EN_USB			0
#define EN_BMP280		0
#define EN_DS18B20		0
#define EN_OLED			0
//#define EN_SPI			0
#define EN_WS2812		1

#if (defined(EN_BMP280) && EN_BMP280==1) || (defined(EN_OLED) && EN_OLED==1)
  #define EN_I2C	1
#else
  #define EN_I2C	0
#endif


//volatile uint32_t status = 0;
//uint32_t source[ARRAYSIZE];
//uint32_t destination[ARRAYSIZE];

//extern uint32_t count_in;
//extern __IO uint32_t count_out;
//extern uint8_t buffer_in[VIRTUAL_COM_PORT_DATA_SIZE];
//extern uint8_t buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];
extern uint32_t _isr_vectorsflash_offs;

static void Ext_Pwr_On(void);
static void Ext_Pwr_Off(void);
static void Ext_Pwr_Init(void);
static void RCC_Configuration(void);
static void I2C1_init(void);
static void NVIC_Configuration(void);


//******************************************************************
// main
//******************************************************************
int main(void)
{
//	register int i;
//	uint32_t val1, val2;
	char buff[80];
	
	// system comes up running HSI at internal RC frequency of 8MHz

#if 1
	SystemCoreClock = 72000000;
		
#if EN_USB
    /* System Clocks Configuration */
    RCC_Configuration();

    NVIC_Configuration();

    // NVIC configuration
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#else
	// Set clock
	SetSysClockTo72();			// SystemCoreClock
	SystemCoreClockUpdate();	// this should always be called after the above
#endif
#endif
	
	// SysTick Init
	sysTickInit();

#if EN_USB
    // enable usb 
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
#endif

#if EN_USB
    // Configure USB pull-up pin
    // ???necessary???
	GPIOC->ODR ^= GPIO_Pin_11;
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //GPIO_Mode_Out_OD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	sysTickDelay(50);
	GPIOC->ODR &= ~GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

	//initialize led
	ledInit();
	ledOff();

#if EN_USB
	USB_Init();
#endif
	
	// Power control to external peripherals
	Ext_Pwr_Init();
	Ext_Pwr_On();	// enable power to external peripherals
	sysTickDelay(100);
	
#if 1
    // Initialize USART
    usart_init();
    uart1_puts("USART1 initialized.\n\r");
//	  write(1, "My name is Kent Larsen\n", 23);
#endif

#if EN_I2C
	I2C1_init();
#endif

#if EN_OLED
	oledInit(I2C1, OLED_ADDR, OLED_128x64, TRUE, FALSE);
	oledFill(0);
#endif

#if EN_USB
	extern USB_Dev_Desc_t dev1;
	printf("Virtual_Com_Port_DeviceDescriptor:\n");
	for (int j = 0; j < sizeof(Virtual_Com_Port_DeviceDescriptor); j++) {
		printf("%02X ", Virtual_Com_Port_DeviceDescriptor[j]);
	}
	printf("\n");
	printf("dev1:\n");
	for (int j = 0; j < sizeof(USB_Dev_Desc_t); j++) {
		printf("%02X ", *((uint8_t *)&dev1 + j));
	}
	printf("\n");
#endif

#if EN_WS2812
	wsInit();
	wsClear();
	wsShow();
	sysTickDelay(200);
//    wsSetPixelColor_rgbw(0, 16, 0, 0, 0);
#endif

//#if EN_SPI
//	spiDmaInit();
//#endif

#if EN_DS18B20
	setup_delay_timer(TIM2);
	delay_us(TIM2, 100);
#endif

#if 0
	// I2C scanner
//	twiEnable();
	printf("Scanning I2C bus...\n");
	twiScanPretty();
#endif

#if EN_BMP280
	int id = BMP280_Init();
//	printf("made it to here\n");
	id = id;
//	printf("ID:     %2x\n", id); sysTickDelay(2);
#endif

	
#if 0
	//initialize source and destination arrays
	for (i = 0; i < ARRAYSIZE; i++) {
		source[i] = i;
		destination[i] = 0;
	}
#endif
	
//	val1 = SysTick->VAL;	
	// Initialize DMA channel
//	dmaInit(source, destination, ARRAYSIZE);

	//LED on before transfer
//	ledOn();

	//Enable DMA1 Channel transfer
//	DMA_Cmd(DMA1_Channel1, ENABLE);

	//wait for DMA transfer to be finished
	// Now go into stop mode, wake up on interrupt
//	asm("    wfi");
	
//	val2 = SysTick->VAL;	
//	printf("dma xfer dt: %ld\n", (val1-val2)/72);
	
//	ledOff();
//	ledOn();
	
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
	
//	ledOff();

#if EN_DS18B20
	sysTickDelay(5);
	ds18b20_init(ONEWIRE_PORT, ONEWIRE_PIN, TIM2);
#endif
	uint32_t loopCnt = 1;
	while (1) {

#if EN_BMP280
		long temperature, ti, tf;
		unsigned long pressure, pi, pf;
			
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

#if EN_OLED
		sprintf(buff, "%d.%02d'C", (int)ti, (int)tf);
		oledWriteString(0, 3, buff, FONT_NORMAL);
		sprintf(buff, "%d.%02d Pa", (int)pi, (int)pf);
		oledWriteString(0, 5, buff, FONT_NORMAL);
#endif
		printf("bmp280:  %d.%02d'C,  %d.%02d hPa\n", (int)ti, (int)tf, (int)pi, (int)pf);
#endif

#if EN_DS18B20
		simple_float temp;
		do {
			temp = ds18b20_get_temperature_simple();
		} while (!temp.is_valid);
#if EN_OLED
		sprintf(buff, "%d.%03d'C", temp.integer, temp.fractional);
		oledWriteString(0, 1, buff, FONT_NORMAL);
#endif
		printf("ds18b20: %d.%03d'C\n", temp.integer, temp.fractional);
#endif

#if EN_WS2812
    	uint32_t c1, c2, c3, c4;
    	c1 = wsColor_rgbw(0, 20, 0, 0);
    	c2 = wsColor_rgbw(0, 8, 0, 0);
    	c3 = wsColor_rgbw(0, 3, 0, 0);
    	c4 = wsColor_rgbw(0, 0, 0, 0);
    	wsTail(c1, c2, c3, c4);
    	wsShow();
    	sysTickDelay(60);
#if 0
    	//wsShow();
    	uint32_t tcolor;

    	tcolor = wsColor_rgbw(16, 0, 0, 0);
		for (int k = 0; k < 64; k++) {
		   	wsChaser(tcolor);
			wsShow();
			sysTickDelay(20);
		}
    	tcolor = wsColor_rgbw(0, 16, 0, 0);
		for (int k = 0; k < 64; k++) {
		   	wsChaser(tcolor);
			wsShow();
			sysTickDelay(20);
		}
    	tcolor = wsColor_rgbw(0, 0, 32, 0);
		for (int k = 0; k < 64; k++) {
		   	wsChaser(tcolor);
			wsShow();
			sysTickDelay(20);
		}
    	tcolor = wsColor_rgbw(10, 10, 10, 0);
		for (int k = 0; k < 64; k++) {
		   	wsChaser(tcolor);
			wsShow();
			sysTickDelay(20);
		}
#endif
#if 0
		//extern uint8_t	spi_tx_buff[];
		///spiDmaTest(&spi_tx_buff[1], 10);
		switch(loopCnt&3) {
		case 0:
    		//wsSetPixelColor_rgbw(0, 16, 0, 0, 0);
    		tcolor = wsColor_rgbw(16, 0, 0, 0);
    		//wsSetPixelColor_c(0, tcolor);
    		wsColorWipe(tcolor, 0);
    		break;
		case 1:
    		//wsSetPixelColor_rgbw(0, 0, 16, 0, 0);
    		tcolor = wsColor_rgbw(0, 16, 0, 0);
    		//wsSetPixelColor_c(0, tcolor);
    		wsColorWipe(tcolor, 0);
    		break;
		case 2:
    		//wsSetPixelColor_rgbw(0, 0, 0, 16, 0);
    		tcolor = wsColor_rgbw(0, 0, 16, 0);
    		//wsSetPixelColor_c(0, tcolor);
    		wsColorWipe(tcolor, 0);
    		break;
		case 3:
    		//wsSetPixelColor_rgbw(0, 16, 16, 16, 32);
    		tcolor = wsColor_rgbw(16, 16, 16, 32);
    		//wsSetPixelColor_c(0, tcolor);
    		wsColorWipe(tcolor, 0);
    		break;
    	}
#endif
#endif

#if EN_USB
        memcpy(buffer_in, "Hello, world!\n\r", 15);
        if (bDeviceState == CONFIGURED) {
        	///memcpy(buffer_in, "Hello, world!\n\r", 15);
			USB_SIL_Write(EP1_IN, buffer_in, 15);
			SetEPTxValid(ENDP1);
        }
#endif
		
#if 0
		ledOn();
		sysTickDelay(50);
		ledOff();
		sysTickDelay(950);
#endif
    	
		loopCnt++;
	}
}

#if EN_USB
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}
#endif

//******************************************************************
// External Power On
//******************************************************************
static void Ext_Pwr_On(void)
{
	GPIO_WriteBit(PWR_PIN_PORT, PWR_PIN, 0);	// enb pwr to ext devices
}

//******************************************************************
// External Power Off
//******************************************************************
static void Ext_Pwr_Off(void)
{
	GPIO_WriteBit(PWR_PIN_PORT, PWR_PIN, 1);	// enb pwr to ext devices
}

//******************************************************************
// External Power Initialize
//******************************************************************
static void Ext_Pwr_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure the PWR pin */
	GPIO_InitStructure.GPIO_Pin = PWR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(PWR_PIN_PORT, &GPIO_InitStructure);

	Ext_Pwr_Off();

}

static void RCC_Configuration(void)
{
    SystemInit();
///	SystemCoreClockUpdate();	// this should always be called after the above
    SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

    // enable usb 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

    // Enable GPIO modules 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC| RCC_APB2Periph_AFIO, ENABLE);

    // RTC clock enable
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
}

//******************************************************************
// I2C
//******************************************************************
static void I2C1_init(void)
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

#if 1
static void NVIC_Configuration(void)
{
    /* Set the Vector Table base location at 0x08000000+_isr_vectorsflash_offs */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_isr_vectorsflash_offs);
}
#endif




