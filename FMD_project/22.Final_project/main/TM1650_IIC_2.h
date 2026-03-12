#ifndef	_TM1650_IIC_2_H_
#define	_TM1650_IIC_2_H_

void IIC_WrByte_TM1650_2(unsigned char txd);
void I2C_Start_TM1650_2(void);
void I2C_Stop_TM1650_2(void);
unsigned char TM1650_2_IIC_wait_ack(void);
void TM1650_2_cfg_display(unsigned char param);
void TM1650_2_Set(unsigned char add, unsigned char dat);
void TM1650_2_Init(void);


#endif