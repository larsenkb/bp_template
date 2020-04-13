#ifndef __TWI_H__
#define __TWI_H__

#include <stdint.h>

extern uint8_t  address, command, length; 
extern uint8_t  buffer[];
extern uint8_t  status1;
extern uint8_t  status2;

void twiEnable(void);
void twiScan(void);
void twiScanPretty(void);
void twiSend(uint8_t address, uint8_t command, uint8_t length);
void twiReceive(uint8_t address, uint8_t command, uint8_t length);


#endif	// __TWI_H__
