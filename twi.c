/* Two Wire Interface, I2C (or IIC), here will be called 'twi', and we have
   only twiEnable(), twiSend() and twiReceive(). The twiSend() function is 
	 fairly simple, we just send address of the device shifted to the left by
	 1 bit, or-red | zero (0) at free space that tell I2C bus it is for write operation.
	 The receive twiReceive() function works by sending address also shifted left
	 one bit with logic or | zero (0) at empty bit (LSB), but then we must send command 
	 to the device depending what device has. After command, we stop (although
	 we can remove STOP condition and continue to "repeated start", then we
	 must change bit after address of the device, now it is one (1) that tells
	 I2C bus we want to read. If we try only read from some address, device
	 don't know what to send. So we must first issue command, then read. For
	 specific command set read datasheet of particular device - it is different
	 for all different devices. More on my website: http://wp.me/p7jxwp-nD
*/

#include <stm32f10x.h>
#include <stdio.h>
#include "utils.h"
#include "twi.h"
#include "systickdelay.h"

void twiEnable(void) 
{
	//just set all registries, but NOT START condition - execute once in main.c
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN; //B port enabled, alternate function 
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; //I2C 1 enabled 
	GPIOB->CRL = 0xFF000000;// setting just pins B7 (SDA) and B6 (SCL), while leaving the rest intact 50 MHz!
	I2C1->CR2 |= 50; // GPIO clock freq=50 MHz MUST !!! be equal APB frequency (GPIO, 2, 10 or 50 MHz)
	I2C1->CCR |= I2C_CCR_FS; //fast mode
	I2C1->CCR |= 30; //not sure for 400 000 - (10= 1.2 MHz, 15=800 kHz, 30=400 kHz)
	I2C1->TRISE |= 51; // maximum rise time is 1000 nS
	I2C1->CR1 |= I2C_CR1_PE; 
}

void twiScan(void)
{
	int a = 0; 
	
	for (uint8_t i = 0; i < 128; i++) {
		I2C1->CR1 |= I2C_CR1_START;
		while(!(I2C1->SR1 & I2C_SR1_SB));
		I2C1->DR = (i << 1 | 0); 
		while(!(I2C1->SR1) | !(I2C1->SR2)) {}; 
		I2C1->CR1 |= I2C_CR1_STOP; 
		delay_us(TIM2, 100);//minium wait time is 40 uS, but for sure, leave it 100 uS
		a = (I2C1->SR1 & I2C_SR1_ADDR);
		if (a == 2) {
			printf("Found I2C device at address 0x%X, or %d (decimal)\n",i,i);
		}
	}
}

void twiScanPretty(void)
{	
	uint8_t address, j;
	int error;
	
	printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	for (address = 0; address < 128; ) {
		printf("%02X ", address);
    	for (j = 0; j < 16; j++) {
			if (address == 0 || address == 127) {
				error = 1;
			} else {
				I2C1->CR1 |= I2C_CR1_START;
				while(!(I2C1->SR1 & I2C_SR1_SB));
				I2C1->DR = ( address << 1 | 0); 
				while(!(I2C1->SR1) | !(I2C1->SR2)) {}; 
				I2C1->CR1 |= I2C_CR1_STOP; 
				//minium wait time is 40 uS, but for sure, leave it 100 uS
				delay_us(TIM2, 100);
				error = (I2C1->SR1 & I2C_SR1_ADDR);
			}
			if (error == 2) {
				printf(" %02X", address);
			} else if (error == 1) {
				printf("   ");
			} else if (error == 4) {
				printf("  E");
			} else {
				printf("  -");
			}
			address++;
		}
		printf("\n");
		sysTickDelay(5);
	}
	printf("\n");
}
	
/* Command or commands, or sending bytes, just the same name of the variable 'command' */
void twiSend(uint8_t address, uint8_t command, uint8_t length)
{
	I2C1->CR1 |= I2C_CR1_START; //START condition 
	while(!(I2C1->SR1 & I2C_SR1_SB));
	
	I2C1->DR = (address << 1 | 0); //sending address of the device, 0 = sending
	while(!(I2C1->SR1 & I2C_SR1_ADDR) | !(I2C1->SR2));		

	I2C1->DR = command; //filling data register with byte, if single - command, multiple - command(s) and data
	for (uint8_t i = 0; i < length; i++) { 
		I2C1->DR = buffer[i]; //filling buffer with command or data
		delay_us(TIM2, 60);
	}
	I2C1->CR1 |= I2C_CR1_STOP;
}

void twiReceive(uint8_t address, uint8_t command, uint8_t length) 
{
	I2C1->CR1 |= I2C_CR1_ACK;
	I2C1->CR1 |= I2C_CR1_START; //start pulse 
	while(!(I2C1->SR1 & I2C_SR1_SB));

	I2C1->DR = (address << 1 | 0); //sending address of the device, 0 = sending
	while(!(I2C1->SR1 & I2C_SR1_ADDR) | !(I2C1->SR2 & I2C_SR2_BUSY));

	I2C1->DR = command; //sending command to the device in order to request data
	I2C1->CR1 |= I2C_CR1_START; //REPEATED START condition to change from sending address + command to receive data
	while(!(I2C1->SR1 & I2C_SR1_SB));

	I2C1->DR = (address << 1 | 1); //sending address of the device, 1 = reading 
	while(!(I2C1->SR1 & I2C_SR1_ADDR) | !(I2C1->SR2));
	
	if (length == 1) { //receiving single byte, N=1
		while(!(I2C1->SR1) | !(I2C1->SR2));
		I2C1->CR1 &= ~I2C_CR1_ACK; //this will send later NAK (not acknowledged) to signal it is last byte
		I2C1->CR1 |= I2C_CR1_STOP; //issuing STOP condition before (!) reading byte
		buffer[0] = I2C1->DR; //single byte is read AFTER NAK (!) and STOP condition
	} 
	if (length == 2) { //receiving two bytes, N=2
		while(!(I2C1->SR1) | !(I2C1->SR2));
		I2C1->CR1 &= ~I2C_CR1_ACK; //this will send later NAK (not acknowledged) before last byte
		I2C1->CR1 |= I2C_CR1_STOP;
		buffer[0] = I2C1->DR; //reading N-1 byte, next to last byte is in DR, last one still in shift register
		while(!(I2C1->SR1 & I2C_SR1_RXNE) | !(I2C1->SR2));
		buffer[1] = I2C1->DR; //read last N byte now available 
	} 
	if (length > 2) { //receiving more than two bytes, N>2
		for (uint8_t i = 0; i < length; i++) { 
			if (i < (length - 3)) {     // if it is not N-2, then read all bytes
				while(!(I2C1->SR1 & I2C_SR1_RXNE) | !(I2C1->SR2));
				buffer[i] = I2C1->DR;  
			} else if (i == (length - 3)) { // if it is N-2 then read 
				while(!(I2C1->SR1) | !(I2C1->SR2));
				buffer[i] = I2C1->DR; 
				while(!(I2C1->SR1 & I2C_SR1_RXNE) | !(I2C1->SR2));
				I2C1->CR1 &= ~I2C_CR1_ACK; //this will send later NAK (not acknowledged) before last byte
				I2C1->CR1 |= I2C_CR1_STOP;
			} else if (i == (length - 2)) { // if it is N-1 then read
				while(!(I2C1->SR1 & I2C_SR1_RXNE) | !(I2C1->SR2));
				buffer[i] = I2C1->DR; 
			} else if (i == (length - 1)) { // else it is N byte 
				while(!(I2C1->SR1 & I2C_SR1_RXNE) | !(I2C1->SR2)) {};
				buffer[i] = I2C1->DR;  
			}
		} 
	}
}
