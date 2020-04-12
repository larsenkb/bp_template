#ifndef __LED_H__
#define __LED_H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"


void ledInit(void);

void ledOn(void);

void ledOff(void);

void ledToggle(void);

#endif   /*  __LED_H__  */
