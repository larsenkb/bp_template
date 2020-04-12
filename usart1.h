#ifndef __USART1_H__
#define __USART1_H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include <stddef.h>

// UART1 uses PA9  as TX
// UART1 uses PA10 as RX

#define TXBUF_PWR	7		// the buffer will be 2**TXBUF_PWR in size
#define RXBUF_PWR	7		// the buffer will be 2**RXBUF_PWR in size
#define RXBUF_SZ	(1<<RXBUF_PWR)
#define TXBUF_SZ	(1<<TXBUF_PWR)
#define RXBUF_MASK	(RXBUF_SZ - 1)
#define TXBUF_MASK	(TXBUF_SZ - 1)

extern char rxBuf[];
extern char txBuf[];
extern volatile int rxBuf_ridx;
extern volatile int rxBuf_widx;
extern volatile int txBuf_ridx;
extern volatile int txBuf_widx;


void uart1_putc(char ch);
void uart1_PrintChar(char ch);

void uart1_puts(char *ptr);

char uart1_getc(void);

int uart1_getline(char *ptr, int len);

void usart_init(void);

void USART1_IRQHandler(void);

#endif  /*  __USART1_H__  */

