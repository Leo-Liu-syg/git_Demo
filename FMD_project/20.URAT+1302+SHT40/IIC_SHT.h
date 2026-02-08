#ifndef	_IIC_SHT_H_
#define	_IIC_SHT_H_

void IIC_SHT_Start(void);
void IIC_SHT_Write_Byte(unsigned char addr_7bit);
unsigned char IIC_SHT_Ack(void);
void IIC_SHT_Write_Command(unsigned char CM_8_bit);
void IIC_SHT_Stop(void);
void IIC_SHT_Read_Byte(unsigned char addr_7bit);
unsigned int IIC_SHT_Read_6Bytes(unsigned int data_buf[6]);
void SHT_process(void);

#endif