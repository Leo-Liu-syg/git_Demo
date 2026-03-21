
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "TDelay.h"

#define SDA_SHT PC0
#define SCL_SHT PC1
#define S 0
#define H 1

unsigned int sht_data_buf[6] = {0}; // 缓冲区初始化清零
float sht_temperature = 0.0f; // 温度(℃)
float sht_humidity = 0.0f;	  // 湿度(%RH)
unsigned int temp_raw = 0;
unsigned int humi_raw = 0;
volatile unsigned char MODE_SorH = 0;

// 温度传感器
void IIC_SHT_Start(void)
{
	TRISC &= ~(1 << 0); // 输出模式
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
	TRISC &= ~(1 << 0);
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
	TRISC |= 1;			  // 输入
	WPUC |= 1;			  // 弱上拉
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
	TRISC = TRISC & ~(1 << 0); // SDA置为0.输出模式
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
	TRISC &= ~(1 << 0);
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
	TRISC |= (1 << 0);	 // PC0设为输入（方向寄存器置1）
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
		TRISC &= ~(1 << 0); // PC0设为输出
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
		TRISC |= (1 << 0);
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

//温度数据处理
void SHT_Data_process(void)
{
	temp_raw = (sht_data_buf[0] << 8) | sht_data_buf[1]; // 温度原始值（第0、1字节）
	humi_raw = (sht_data_buf[3] << 8) | sht_data_buf[4]; // 湿度原始值（第3、4字节）
	// 摄氏度
	if (MODE_SorH == S)
	{
		sht_temperature = (float)(temp_raw * 175.0f / 65535.0f) - 45.0f;
	} // 温度公式：-45~125℃
	// 华氏度
	if (MODE_SorH == H)
	{
		sht_temperature = (float)(-49.0f + 315.0f * (temp_raw / 65535.0f));
	}
}
