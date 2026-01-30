#ifndef	_TM1650_IIC_H_
#define	_TM1650_IIC_H_

void IIC_WrByte_TM1650(unsigned char txd);
void I2C_Start_TM1650(void);
void I2C_Stop_TM1650(void);
unsigned char TM1650_IIC_wait_ack(void);
void TM1650_cfg_display(unsigned char param);
void TM1650_Set(unsigned char add, unsigned char dat);
void TM1650_DisplayClose(void);
void TM1650_Init(void);


#endif