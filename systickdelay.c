#include "stm32f10x.h"

static volatile uint32_t sysTickCount = 0;

void SysTick_Handler(void)
{
	sysTickCount++;
}

void sysTickInit(void)
{
	SysTick_Config(SystemCoreClock/1000);
	sysTickCount = 0;
}

void sysTickDelay(uint32_t ticks)
{
	uint32_t delay = sysTickCount + ticks;
	
	while (sysTickCount != delay)
		asm("    wfi");
}

void sysTickSet(uint32_t nTime)
{
	sysTickCount = nTime;
}

uint32_t sysTickGet(void)
{
	return sysTickCount;
}

#if 0
void sysTickDalayInit()
{
  SysTick_Config(SystemCoreClock/1000);
}

void sysTickDalay(uint32_t nTime)
{
	sysTickCount = nTime;
	while(sysTickCount != 0);
}
#endif
