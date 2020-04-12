//#include "stm32f10x.h"
//#include "stm32f10x_gpio.h"

#include "led.h"

void ledInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* Initialize LED which connected to PC13 */
	// Enable PORTC Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOC, GPIO_Pin_13); // Set C13 to High level ("1")
}

void ledOn(void)
{
	GPIOC->ODR &= ~GPIO_Pin_13;
}

void ledOff(void)
{
	GPIOC->ODR |= GPIO_Pin_13;
}

void ledToggle(void)
{
	GPIOC->ODR ^= GPIO_Pin_13;
}
