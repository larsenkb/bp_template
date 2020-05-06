#ifndef __UTILS_H__
#define __UTILS_H__
#include <stm32f10x.h>
#include <stm32f10x_bkp.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_pwr.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_rtc.h>
#include <stm32f10x_tim.h>

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define U_ID_PTR            (0x1FFFF7E8)
#define U_ID_0 (*(uint32_t*)(U_ID_PTR))
#define U_ID_1 (*(uint32_t*)(U_ID_PTR + 4))
#define U_ID_2 (*(uint32_t*)(U_ID_PTR + 8))

void hacf(void);
void setup_delay_timer(TIM_TypeDef *timer);
void delay(__IO uint32_t nTime /*ms*/);
void delay_decrement(void);
void delay_us(TIM_TypeDef *timer, unsigned int time);
void delay_ms(TIM_TypeDef *timer, unsigned int time);

uint8_t sadd8(uint8_t a, uint8_t b);
uint16_t sadd16(uint16_t a, uint16_t b);
uint32_t sadd32(uint32_t a, uint32_t b);

#define max(a, b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define min(a, b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })
#endif
