#include "usart1.h"

// UART1 uses PA9  as TX
// UART1 uses PA10 as RX

char rxBuf[RXBUF_SZ];
char txBuf[TXBUF_SZ];
volatile int rxBuf_ridx;
volatile int rxBuf_widx;
volatile int txBuf_ridx;
volatile int txBuf_widx;


void uart1_putc(char ch)
{
	int widx = (txBuf_widx + 1) & TXBUF_MASK;

	if (widx != txBuf_ridx) {
		txBuf[txBuf_widx] = ch;
		txBuf_widx = widx;
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
}


#if 0
void uart1_PrintChar(char ch)
{
//	if (ch == '\n')
//		uart1_putc('\r');
	uart1_putc(ch);
}
#endif

int __io_putchar(char ch)
{
	if (ch == '\n') __io_putchar('\r');
	uart1_putc(ch);
	return (int)ch;
}

void uart1_puts(char *ptr)
{
	while (*ptr)
		uart1_putc(*ptr++);
}

char uart1_getc(void)
{
	char ch;

	while (rxBuf_ridx == rxBuf_widx);
	ch = rxBuf[rxBuf_ridx];
	rxBuf_ridx = (rxBuf_ridx + 1) & RXBUF_MASK;
	return ch;
}

int uart1_getline(char *ptr, int len)
{
	int idx = 0;
	char ch;

	if ((ptr == (char*)NULL) || len < 2)
		return idx;

	len--;
	ptr[0] = '\0';
	while (((ch = uart1_getc()) != '\r') && (idx < len)) {
		if (idx < len) {
			ptr[idx++] = ch;
			ptr[idx] = '\0';
		}
	}
	return idx;
}

void usart_init(void)
{
	rxBuf_ridx = 0;
	rxBuf_widx = 0;
	txBuf_ridx = 0;
	txBuf_widx = 0; 

	/* Enable USART1 and GPIOA clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* NVIC Configuration */
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the GPIOs */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure the USART1 */
	USART_InitTypeDef USART_InitStructure;

	/* USART1 configuration ------------------------------------------------------*/
	/* USART1 configured as follow:
		- BaudRate = 115200 baud
		- Word Length = 8 Bits
		- One Stop Bit
		- No parity
		- Hardware flow control disabled (RTS and CTS signals)
		- Receive and transmit enabled
		- USART Clock disabled
		- USART CPOL: Clock is active low
		- USART CPHA: Data is captured on the middle
		- USART LastBit: The clock pulse of the last data bit is not output to
			the SCLK pin
	 */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);

	/* Enable the USART1 Receive interrupt: this interrupt is generated when the
		USART1 receive data register is not empty */
///	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
///	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void USART1_IRQHandler(void)
{
	int widx;
	char ch;

	if ((USART1->SR & USART_FLAG_RXNE)) { // != (u16)RESET) {
	//if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		//USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		ch = USART_ReceiveData(USART1);
		widx = (rxBuf_widx + 1) & RXBUF_MASK;
		if (widx != rxBuf_ridx) {
			rxBuf[rxBuf_widx] = ch;
			rxBuf_widx = widx;
		}
		uart1_putc(ch);
	}

	if (USART1->CR1 & USART_FLAG_TXE) {  // if txe interrupt enabled...
//	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
		if ((USART1->SR & USART_FLAG_TXE)) {  // if txe is set...
			if (txBuf_ridx != txBuf_widx) {
				USART1->DR = txBuf[txBuf_ridx];
				//USART_SendData(USART1, txBuf[txBuf_ridx]);
				txBuf_ridx = (txBuf_ridx + 1) & TXBUF_MASK;
			} else {
				USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
			}
		}
	}
}


