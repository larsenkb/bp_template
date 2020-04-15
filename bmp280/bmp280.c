#include "stm32f10x_i2c.h"
#include "bmp280.h"
#include <math.h>

unsigned short dig_T1;
short dig_T2;
short dig_T3;
unsigned short dig_P1;

short dig_P2;
short dig_P3;
short dig_P4;
short dig_P5;
short dig_P6;
short dig_P7;
short dig_P8;
short dig_P9;

unsigned char bmp280_id;

unsigned char bmp280ReadByte(unsigned char address)
{
  unsigned char lsb = 0;

  I2C_AcknowledgeConfig(I2C1,ENABLE);
  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Receiver);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  lsb = I2C_ReceiveData(I2C1);

//  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
//  msb = I2C_ReceiveData(I2C1);

  I2C_GenerateSTOP(I2C1,ENABLE);
  I2C_AcknowledgeConfig(I2C1,DISABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  I2C_ReceiveData(I2C1);

  return lsb;
}

short bmp280ReadShort(unsigned char address)
{
  short msb=0;
  short lsb=0;

  I2C_AcknowledgeConfig(I2C1,ENABLE);
  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Receiver);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  lsb = I2C_ReceiveData(I2C1);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  msb = I2C_ReceiveData(I2C1);

  I2C_GenerateSTOP(I2C1,ENABLE);
  I2C_AcknowledgeConfig(I2C1,DISABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  I2C_ReceiveData(I2C1);

  return (msb << 8) | lsb;
}

long bmp280ReadLong(unsigned char address)
{
  long result=0;

  long msb=0;
  long lsb=0;
  long xsb=0;

  I2C_AcknowledgeConfig(I2C1,ENABLE);
  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Receiver);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  msb = (long)I2C_ReceiveData(I2C1);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  lsb = (long)I2C_ReceiveData(I2C1);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  xsb = (long)I2C_ReceiveData(I2C1);

  I2C_GenerateSTOP(I2C1,ENABLE);
  I2C_AcknowledgeConfig(I2C1,DISABLE);
  
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
  I2C_ReceiveData(I2C1);

  result = msb;
  result = (result<<8) | lsb;
  result = (result<<4) | ((xsb>>4) & 0x0f);
  return result;
//  result = (msb << 16) | (lsb << 8) | xsb;
  
  //return result;
//  return (result >> 4);
}

void bmp280WriteByte(unsigned char address, unsigned char data)
{
  I2C_GenerateSTART(I2C1,ENABLE);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(I2C1, BMP280_addr, I2C_Direction_Transmitter);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(I2C1,address);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_SendData(I2C1,data);
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

  I2C_GenerateSTOP(I2C1,ENABLE);

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}
//----------------------------------------
#if 1
long t_fine;
long bmp280_compensate_T(long adc_T)
{
  long var1, var2, T;

  var1 = ((((adc_T>>3) - ((long)dig_T1<<1))) * ((long)dig_T2)) >> 11;
  var2 = (((((adc_T>>4) - ((long)dig_T1)) * ((adc_T>>4) - ((long)dig_T1))) >> 12) * ((long)dig_T3)) >> 14;
  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  return T;
}

unsigned long bmp280_compensate_P(long adc_P)
{
  long long var1, var2, p;

  var1 = ((long long)t_fine) - 128000;
  var2 = var1 * var1 * (long long)dig_P6;
  var2 = var2 + ((var1 * (long long)dig_P5) << 17);
  var2 = var2 + (((long long)dig_P4) << 35);
  var1 = ((var1 * var1 * (long long)dig_P3) >> 8) + ((var1 * (long long)dig_P2) << 12);
  var1 = (((((long long)1)<<47)+var1))*((long long)dig_P1)>>33;
  if (var1 == 0) {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p<<31) - var2) * 3125) / var1;
  var1 = (((long long)dig_P9) * (p>>13) * (p>>13)) >> 25;
  var2 = (((long long)dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((long long)dig_P7) << 4);

  return (unsigned long)p;
}

void bmp280Convert(long *temperature, unsigned long *pressure)
{
  long adc_T;
  long adc_P;

  if (temperature == (long*)NULL) return;
  if (pressure == (unsigned long*)NULL) return;

  adc_T = bmp280ReadLong(BMP280_REG_RESULT_TEMPRERATURE);
  adc_P = bmp280ReadLong(BMP280_REG_RESULT_PRESSURE);
#if 0
  *temperature = adc_T;
  *pressure = adc_P;
#else
  *temperature = bmp280_compensate_T(adc_T);
  *pressure = bmp280_compensate_P(adc_P);
#endif
  return;
}
#else
void bmp280Convert(double* temperature, double* pressure)
{
  unsigned long adc_T;
  unsigned long adc_P;
  adc_T = bmp280ReadLong(BMP280_REG_RESULT_TEMPRERATURE);
  adc_P = bmp280ReadLong(BMP280_REG_RESULT_PRESSURE);

  double var1, var2, p, t_fine;
  var1 = (((double)adc_T)/16384.0 - ((double)dig_T1)/1024.0) * ((double)dig_T2);
  var2 = ((((double)adc_T)/131072.0 - ((double)dig_T1)/8192.0) * (((double)adc_T)/131072.0 - ((double) dig_T1)/8192.0)) * ((double)dig_T3);
  t_fine = (var1 + var2);
  //*temperature = (int32_t) (t_fine*10 / 5120.0);
  if (temperature != (double*)NULL)
    *temperature = t_fine / 5120.0;

  var1 = ((double)t_fine/2.0) - 64000.0;
  var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
  var2 = var2 + var1 * ((double)dig_P5) * 2.0;
  var2 = (var2/4.0)+(((double)dig_P4) * 65536.0);
  var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
  var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);
  if (var1 == 0.0)
  {
    return; // avoid exception caused by division by zero
  }
  p = 1048576.0 - (double)adc_P;
  p = (p - (var2 / 4096.0)) * 6250.0 / var1;
  var1 = ((double)dig_P9) * p * p / 2147483648.0;
  var2 = p * ((double)dig_P8) / 32768.0;
  p = (p + (var1 + var2 + ((double)dig_P7)) / 16.0);

  //*pressure = (long)p;
  if (pressure != (double*)NULL)
    *pressure = p;
}
#endif
//----------------------------------------

int BMP280_Init(void)
{ 
  bmp280_id = bmp280ReadByte(0xD0);//id
  dig_T1 = bmp280ReadShort(0x88);//dig_T1
  dig_T2 = bmp280ReadShort(0x8A);//dig_T2
  dig_T3 = bmp280ReadShort(0x8C);//dig_T3
  dig_P1 = bmp280ReadShort(0x8E);//dig_P1
  dig_P2 = bmp280ReadShort(0x90);//dig_P2
  dig_P3 = bmp280ReadShort(0x92);//dig_P3
  dig_P4 = bmp280ReadShort(0x94);//dig_P4
  dig_P5 = bmp280ReadShort(0x96);//dig_P5
  dig_P6 = bmp280ReadShort(0x98);//dig_P6
  dig_P7 = bmp280ReadShort(0x9A);//dig_P7
  dig_P8 = bmp280ReadShort(0x9C);//dig_P8
  dig_P9 = bmp280ReadShort(0x9E);//dig_P9

  bmp280WriteByte(BMP280_REG_CONFIG, BMP280_CONFIG);
  bmp280WriteByte(BMP280_REG_CONTROL, BMP280_MEAS);
  return (int)bmp280_id;
}

//----------------------------------------

int BMP280_Get(int idx)
{
	switch(idx) {
	case 0: return (int)dig_T1; break;
	case 1: return (int)dig_T2; break;
	case 2: return (int)dig_T3; break;
	case 3: return (int)dig_P1; break;
	case 4: return (int)dig_P2; break;
	case 5: return (int)dig_P3; break;
	case 6: return (int)dig_P4; break;
	case 7: return (int)dig_P5; break;
	case 8: return (int)dig_P6; break;
	case 9: return (int)dig_P7; break;
	case 10: return (int)dig_P8; break;
	case 11: return (int)dig_P9; break;
	}
	return 0;
}
