
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "TDelay.h"

#define SDA_SHT PA4
#define SCL_SHT PA5
unsigned int sht_data_buf[6] = {0}; // 缓冲区初始化清零

// 温度传感器
void IIC_SHT_Start(void)
{
	TRISA &= ~(1 << 4); // 输出模式
	SDA_SHT = 1;
	SCL_SHT = 1;
	TDelay_us(5);
	SDA_SHT = 0;
	TDelay_us(5);
	SCL_SHT = 0;
}

void IIC_SHT_Write_Byte(unsigned char addr_7bit) // 写请求，写0x44或0x45
{
	unsigned char txd;
	txd = (addr_7bit << 1) | 0;
	TRISA &= ~(1 << 4);
	ODCON0 |= (1 << 4); // SDA_seg1线输出模式
	SCL_SHT = 0;
	for (unsigned char i = 0; i < 8; i++)
	{
		SDA_SHT = (txd & 0x80) ? 1 : 0;
		txd <<= 1;
		TDelay_us(2);
		SCL_SHT = 1;
		TDelay_us(2);
		SCL_SHT = 0;
		TDelay_us(2);
	}
}

unsigned char IIC_SHT_Ack(void)
{
	TRISA |= (1 << 4);			  // 输入
	WPUA |= (1 << 4);			  // 弱上拉
	unsigned char Ack_signal = 1; // Default None Ack
	SCL_SHT = 1;
	TDelay_us(2);
	for (int i = 0; i < 3; i++)
	{
		if (SDA_SHT == 0)
		{
			Ack_signal = 0;
			break;
		}
		TDelay_us(1);
	}
	SCL_SHT = 0;
	TDelay_us(5);
	return Ack_signal;
}

void IIC_SHT_Write_Command(unsigned char CM_8_bit) // 选择精度(0xFD)
{
	unsigned char txd;
	unsigned char i;
	TRISA = TRISA & ~(1 << 4); // SDA置为0.输出模式

	txd = CM_8_bit;
	SCL_SHT = 0;
	for (i = 0; i < 8; i++)
	{
		if (txd & 0x80)
			SDA_SHT = 1;
		else
			SDA_SHT = 0;
		txd <<= 1;
		TDelay_us(2);
		SCL_SHT = 1;
		TDelay_us(2);
		SCL_SHT = 0;
		TDelay_us(2);
	}
}

void IIC_SHT_Stop(void)
{
	TRISA &= ~(1 << 4);
	SDA_SHT = 0;
	SCL_SHT = 0;
	TDelay_us(2);
	SCL_SHT = 1;
	TDelay_us(5);
	SDA_SHT = 1;
	TDelay_us(2);
}

void IIC_SHT_Read_Byte(unsigned char addr_7bit) // 读请求，读0x44或0x45
{
	unsigned char txd;
	unsigned char i;
	txd = (addr_7bit << 1) | 1;
	ODCON0 |= 0B00010000; // SDA_seg1线输出模式
	SCL_SHT = 0;
	for (i = 0; i < 8; i++)
	{
		if (txd & 0x80)
			SDA_SHT = 1;
		else
			SDA_SHT = 0;
		txd = txd << 1;
		TDelay_us(2);
		SCL_SHT = 1;
		TDelay_us(2);
		SCL_SHT = 0;
		TDelay_us(2);
	}
}

// 读取6字节传感器数据（含CRC校验）
unsigned int IIC_SHT_Read_6Bytes(unsigned int data_buf[6])
{
	// --------------------------
	// 1. SDA切换为输入模式，准备读取
	// --------------------------
	TRISA |= (1 << 4);	 // PA4设为输入（方向寄存器置1）
	ODCON0 &= ~(1 << 4); // 关闭SDA输出驱动器（输入模式不需要）
	SCL_SHT = 0;		 // SCL初始拉低

	// --------------------------
	// 2. 循环读取6字节数据
	// --------------------------
	for (int byte_idx = 0; byte_idx < 6; byte_idx++)
	{
		unsigned int recv_byte = 0;

		// --------------------------
		// 3. 逐位读取1字节（高位→低位）
		// --------------------------
		for (int bit_idx = 0; bit_idx < 8; bit_idx++)
		{
			// 拉高SCL ≥2us，等待传感器输出数据
			SCL_SHT = 1;
			TDelay_us(2);

			// 读取SDA电平，拼接到当前字节（高位在前）
			recv_byte = (recv_byte << 1) | (SDA_SHT == 1 ? 1 : 0);

			// 拉低SCL，准备下一位
			SCL_SHT = 0;
			TDelay_us(2);
		}
		data_buf[byte_idx] = recv_byte; // 保存当前字节

		// --------------------------
		// 4. 发送ACK/NACK

		// --------------------------
		// 临时切换SDA为输出模式（发送ACK/NACK需要主机输出）
		TRISA &= ~(1 << 4); // PA4设为输出
		ODCON0 |= (1 << 4); // 使能输出驱动器

		if (byte_idx < 5)
		{
			// 前5字节：发送ACK（SDA拉低）
			SDA_SHT = 0;
		}
		else
		{
			// 第6字节（最后1个）：发送NACK（SDA拉高）
			SDA_SHT = 1;
		}

		// ACK/NACK时序：拉高SCL≥2us → 拉低SCL
		SCL_SHT = 1;
		TDelay_us(2);
		SCL_SHT = 0;
		TDelay_us(2);

		// 切换SDA回输入模式，准备读取下一字节
		TRISA |= (1 << 4);
		ODCON0 &= ~(1 << 4);
	}

	return 0; // 读取完成
}

void SHT_process(void)
{
	unsigned char ack;
	IIC_SHT_Start();
	IIC_SHT_Write_Byte(0x44);
	ack = IIC_SHT_Ack();
	if (ack == 1)
	{
		IIC_SHT_Stop();
		return; // 无ACK，终止流程
	}
	IIC_SHT_Write_Command(0xFD);
	ack = IIC_SHT_Ack();
	if (ack == 1)
	{
		IIC_SHT_Stop();
		return; // 无ACK，终止流程
	} // NACK则退出
	IIC_SHT_Stop();

	TDelay_ms(18); // 等待传感器完成传递数据

	IIC_SHT_Start();
	IIC_SHT_Read_Byte(0x44); // 包含了ack
	ack = IIC_SHT_Ack();
	if (ack == 1)
	{
		IIC_SHT_Stop();
		return; // 无ACK，终止流程
	}
	IIC_SHT_Read_6Bytes(sht_data_buf);
	IIC_SHT_Stop();
}

