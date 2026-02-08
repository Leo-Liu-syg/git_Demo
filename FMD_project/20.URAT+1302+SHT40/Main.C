// Project: TEST_64F0Ax_EEPROM.prj
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128
// Description:
// 1.电脑串口发送 0xAA 然后 EEPROM地址 EEPROM值，芯片收到之后在对应的EEPROM地址上写值
// 2.电脑串口发送 0xBB 然后 EEPROM地址，芯片收到之后在读的EEPROM地址上的值然后返回给电脑
//
// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.6        25-6-4        修改系统时钟为8MHz，使能LVR
//
//                FT64F0A5  TSSOP20
//              -------------------
// NC----------|1(PA5)   	(PA4)20|-----------NC
// NC----------|2(PA6)   	(PA3)19|-----------NC
// NC----------|3(PA7)   	(PA2)18|-----------NC
// NC----------|4(PC0)   	(PA1)17|-----------NC
// NC----------|5(PC1)		(PA0)16|-----------NC
// NC----------|6(PB7)		(PB0)15|-----------NC
// GND---------|7(GND)		(PB1)14|-----------NC
// NC----------|8(PB6)		(PB2)13|-----------NC
// VDD---------|9(VDD)		(PB3)12|-----------NC
// NC----------|10(PB5)		(PB4)11|-----------NC
//				-------------------
//
//*********************************************************
#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "URAT_INITIAL.h"
#include "TDelay.h"
#include "Urat_as_Printf.h"
#include "eeprom.h"
#include "DS1302.h"
#include "IIC_SHT.h"
//***********************宏定义****************************
// #define		uchar	unsigned char
// 1. 引脚宏定义（依据：手册2.3.3 TRISB寄存器、2.3.6 PORTB寄存器）
#define DS1302_SCLK_PIN 0 // PB0
#define DS1302_DATA_PIN 1 // PB1
#define DS1302_RST_PIN 2  // PB2

// 温湿度
#define SDA_SHT PA4
#define SCL_SHT PA5

// 时间变量定义
unsigned char ds1302_year = 26;	  // 默认2024年
unsigned char ds1302_month = 12;  // 默认12月
unsigned char ds1302_day = 31;	  // 默认31日
unsigned char ds1302_hour = 23;	  // 默认23时
unsigned char ds1302_minute = 59; // 默认59分
unsigned char ds1302_second = 0;  // 默认0秒
unsigned char ds1302_week = 7;	  // 默认周六（7）

unsigned char Update_Time_Flag = 0;
unsigned char Read_Time_Flag = 0;
// 温湿度
volatile unsigned char Tem = 0;
volatile unsigned char Hum = 0;
volatile unsigned char Tem_For_dis = 0;
volatile unsigned char Hum_For_dis = 0;
unsigned char SHT_Flag = 0;
//  定义全局/局部缓冲区（6个元素，存传感器返回的6字节原始数据）
unsigned int sht_data_buf[6] = {0}; // 缓冲区初始化清零
//  定义变量存最终实际温湿度（浮点型，保留2位小数足够）
float sht_temperature = 0.0f; // 温度(℃)
float sht_humidity = 0.0f;	  // 湿度(%RH)
unsigned int temp_raw = 0;
unsigned int humi_raw = 0;
unsigned char MODE_SorH = 0;
// EEPROM相关定义
// 规定
volatile char W_TMP @0x70;	 // 系统占用不可以删除和修改
volatile char BSR_TMP @0x71; // 系统占用不可以删除和修改
volatile unsigned char Receivedata[] = {0x13, 0x25, 0x77, 0x25, 0x25, 0x25, 0x25};
volatile unsigned char Room_number[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
volatile unsigned char SHT_Data[] = {0x00, 0x00};
// aa写入0，0x11房间号1（年），0x12房间号2（月），0x13房间号3（日），0x14房间号4（时），0x15房间号5（分），0x16房间号6（秒）
volatile unsigned char Read_Year = 0;
volatile unsigned char Read_Month = 0;
volatile unsigned char Read_Date = 0;
volatile unsigned char Read_Hour = 0;
volatile unsigned char Read_Minute = 0;
volatile unsigned char Read_Second = 0;


volatile unsigned char senddata[] = 0;
volatile unsigned char EEReadData = 0;
volatile unsigned char i;
volatile unsigned char j;
unsigned char send_flag = 0;
unsigned char receiveddone_flag = 0;
unsigned char init_send_done = 0;
void user_isr(); // 用户中断程序，不可删除
unsigned char EEPROMread(unsigned char EEAddr);
void EEPROMwrite(unsigned char EEAddr, unsigned char Data);

//===========================================================
// Funtion name：interrupt ISR
// parameters：无
// returned value：无
//===========================================================
void interrupt ISR(void)
{
#asm;			// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
	NOP;		// 系统设置不可以删除和修改
#endasm;		// 系统设置不可以删除和修改
	user_isr(); // 调用用户中断函数
}
void user_isr() // 调用用户中断函数
{
	if (UR1RXNE && UR1RXNEF) // 串口接收开始
	{
		UR1RXNEF = 0;
		Receivedata[0] = UR1DATAL;
		SHT_Flag = 1;				// 时不时读一下温湿度
		if (Receivedata[0] == 0xAA) // 显示保存的值
		{
			// for (unsigned char u = 1; u < 7; u++)
			// {
			// 	while (!UR1RXNEF)
			// 		; // 硬件自动置1后跳出
			// 	UR1RXNEF = 0;
			// 	Room_number[u] = UR1DATAL;
			// }
			// Update_Time_Flag = 1;
			receiveddone_flag = 1;
		}
		else if (Receivedata[0] == 0xBB) // 写指令
		{
			EEPROMwrite(0x20, Tem);
			EEPROMwrite(0x21, Hum);
			EEPROMwrite(0x11, Receivedata[1]); // 写入ROM
			EEPROMwrite(0x12, Receivedata[2]); // 写入ROM
			EEPROMwrite(0x13, Receivedata[3]); // 写入ROM
			EEPROMwrite(0x14, Receivedata[4]); // 写入ROM
			EEPROMwrite(0x15, Receivedata[5]); // 写入ROM
			EEPROMwrite(0x16, Receivedata[6]); // 写入ROM
			Update_Time_Flag = 1;			   // 置1，在main里更新时间然后写到芯片
			receiveddone_flag = 1;
		}
		else if (Receivedata[0] == 0xCC)
		{
			for (unsigned char u = 1; u < 7; u++)
			{
				while (!UR1RXNEF)
					; // 硬件自动置1后跳出
				UR1RXNEF = 0;
				Receivedata[u] = UR1DATAL; // 把设置的值都写到receive data[]里面
			} // 年

			receiveddone_flag = 1;
		}
		// 接受完成标志
		if (receiveddone_flag == 1)
		{
			if (Receivedata[0] == 0xAA)
			{
				UART_SendString("\r\n[Command AA]");
				receiveddone_flag = 0; // 置零，以按顺序串口发送
			}
			if (Receivedata[0] == 0xBB)
			{
				UART_SendString("\r\n[Command BB: save all data in EEPROM]");
				receiveddone_flag = 0; // 置零，以按顺序串口发送
			}
			if (Receivedata[0] == 0xCC)
			{
				UART_SendString("\r\n[Command CC:Reset DS1302 Time]");
				receiveddone_flag = 0; // 置零，以按顺序串口发送
			}
			send_flag = 1;
		}
	}

	// 串口发送回电脑
	if (UR1TCEN && UR1TCF)
	{
		UR1TCF = 0;
		if (send_flag == 1 && receiveddone_flag == 0)
		{
			if (Receivedata[0] == 0xAA) // 显示年月日时分秒 温湿度
			{
				UART_SendString("\r\n[Current Time]: ");
				for (j = 1; j < 7; j++)
				{
					senddata[j] = EEPROMread(Room_number[j]);
					UART_SendHex(senddata[j]);
					if (j >= 1 && j <= 3)
					{
						UART_SendString("/");
					}
					else if (j > 3 && j < 6)
					{
						UART_SendString(":");
					}
					else if (j == 6)
					{
					}
				}
				UART_SendString("\r\n[Current Tem]: ");
				Tem_For_dis = EEPROMread(0x20);
				UART_SendHex(Tem_For_dis);
				UART_SendString("\r\n[Current Hum]: ");
				Hum_For_dis = EEPROMread(0x21);
				UART_SendHex(Hum_For_dis);
			}
			if (Receivedata[0] == 0xBB)
			{
				UART_SendString("\r\n[finished BB: save all data in EEPROM]");
			}
			if (Receivedata[0] == 0xCC)
			{
				UART_SendString("\r\n[Finished CC:Reset DS1302 Time]");
			}
			send_flag = 0;
		}
	}
}
/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // 系统时钟选择为内部振荡器16MHz,分频比为2:1
	INTCON = 0;			 // 禁止所有中断

	PORTA = 0B00000000; // PA7 芯片rx
	PORTB = 0B00000000;
	PORTC = 0B00000000;

	WPUA = 0B00000000; // 弱上拉的开关，0-关，1-开
	WPUB = 0B00000000;
	WPUC = 0B00000000;

	WPDA = 0B00000000; // 弱下拉的开关，0-关，1-开
	WPDB = 0B00000000;
	WPDC = 0B00000000;

	TRISA = 0B10000000; // 输入输出设置，0-输出，1-输入
	TRISB = 0B00000000;
	TRISC = 0B00000000;

	PSRC0 = 0B11111111; // 源电流设置最大
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;

	PSINK0 = 0B11111111; // 灌电流设置最大
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;

	ANSELA = 0B00000000; // 数字IO
}

/*-------------------------------------------------
 * 函数名：main
 * 功能：	 主函数
 * 输入：	 无
 * 输出： 	 无
 --------------------------------------------------*/
void main(void)
{
	POWER_INITIAL(); // 系统初始化
	DS1302_Init();
	SHT_process();				   // 上电测一次
	EEPROMwrite(0x19, 0x55);	   // 0x55写入地址0x13
	EEReadData = EEPROMread(0x19); // 芯片上电的时候把EEPROM 0x13房间里的内容通过串口显示出来

	SHT_process();
	temp_raw = (sht_data_buf[0] << 8) | sht_data_buf[1]; // 温度原始值（第0、1字节）
	humi_raw = (sht_data_buf[3] << 8) | sht_data_buf[4]; // 湿度原始值（第3、4字节）
	sht_temperature = (float)(temp_raw * 175.0f / 65535.0f) - 45.0f;
	sht_humidity = (float)(humi_raw * 100.0f / 65535.0f); // 湿度公式：0~100%RH
	Tem = sht_temperature;
	Hum = sht_humidity;

	Receivedata[1] = EEPROMread(0x11);
	Receivedata[2] = EEPROMread(0x12);
	Receivedata[3] = EEPROMread(0x13);
	Receivedata[4] = EEPROMread(0x14);
	Receivedata[5] = EEPROMread(0x15);
	Receivedata[6] = EEPROMread(0x16);

	ds1302_year = (Receivedata[1] / 16) * 10 + Receivedata[1] % 16; // 26
	ds1302_month = (Receivedata[2] / 16) * 10 + Receivedata[2] % 16;
	ds1302_day = (Receivedata[3] / 16) * 10 + Receivedata[3] % 16;
	ds1302_hour = (Receivedata[4] / 16) * 10 + Receivedata[4] % 16;
	ds1302_minute = (Receivedata[5] / 16) * 10 + Receivedata[5] % 16;
	ds1302_second = (Receivedata[6] / 16) * 10 + Receivedata[6] % 16;

	UART_INITIAL(); // 使能串口，目前来看必须在write弄完了之后再开启中断，否则会无法解锁
	DelayMs(50);	// 延迟一会，等待串口ok
	DS1302_WriteTime();
	UR1TCF = 0;
	UR1DATAL = EEReadData;
	while (1)
	{
		// 已验证能正确传过来

		if (Update_Time_Flag == 1)
		{
			ds1302_year = (Receivedata[1] / 16) * 10 + Receivedata[1] % 16; // 26
			ds1302_month = (Receivedata[2] / 16) * 10 + Receivedata[2] % 16;
			ds1302_day = (Receivedata[3] / 16) * 10 + Receivedata[3] % 16;
			ds1302_hour = (Receivedata[4] / 16) * 10 + Receivedata[4] % 16;
			ds1302_minute = (Receivedata[5] / 16) * 10 + Receivedata[5] % 16;
			ds1302_second = (Receivedata[6] / 16) * 10 + Receivedata[6] % 16;
			// 已实现
			DS1302_WriteTime();
			Update_Time_Flag = 0;
		}

		DS1302_ReadTime();

		if (SHT_Flag == 1)
		{
			SHT_process();
			temp_raw = (sht_data_buf[0] << 8) | sht_data_buf[1]; // 温度原始值（第0、1字节）
			humi_raw = (sht_data_buf[3] << 8) | sht_data_buf[4]; // 湿度原始值（第3、4字节）
			sht_temperature = (float)(temp_raw * 175.0f / 65535.0f) - 45.0f;
			sht_humidity = (float)(humi_raw * 100.0f / 65535.0f); // 湿度公式：0~100%RH
			Tem = sht_temperature;
			Hum = sht_humidity;
			SHT_Flag = 0;
		}
	}
}