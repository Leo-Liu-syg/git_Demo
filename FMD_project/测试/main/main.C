//Project: main.prj
// Device: FT61F0AX
// Memory: Flash 10KX14b, EEPROM 128X8b, SRAM 1KX8b
// Author: 
//Company: 
//Version:
//   Date: 
//===========================================================
//===========================================================
#include	"SYSCFG.h"
#include "FT64F0AX.h"

//***********************宏定义****************************
#define uint unsigned int
#define ulong unsigned long
#define TM1650_WRITE_ADDR 0x48
#define TM1650_CMD_DISP_ON 0x01 // 8???????7???????????????????????????
#define TM1650_CMD_ADDR_BASE 0x68
#define SCL_seg1 PB3
#define SDA_seg1 PB4

#define SCL_seg2 PA0
#define SDA_seg2 PA1

#define SDA_SHT PA4
#define SCL_SHT PA5

#define EC11_A PA7
#define EC11_B PA6

// 数码管变量定义
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};
// 温度
int Number_Sum_1 = 0;
int Number_Sum_1_Old = 0;
int Number_Sum_1_Abs = 0; // 用于处理负数
int Number_Ge_1 = 0;
int Number_Shi_1 = 0;
int Number_Bai_1 = 0;
int Number_Qian_1 = 0;
// 湿度
int Number_Sum_2 = 0;
int Number_Sum_2_Old = 0;
int Number_Sum_2_Abs = 0; // 用于处理负数
int Number_Ge_2 = 0;
int Number_Shi_2 = 0;
int Number_Bai_2 = 0;
int Number_Qian_2 = 0;

// LED_Breath
unsigned char LED_Count = 0;
unsigned char LED_PWM_Mid = 0;
unsigned char Breath_Flag1 = 0;

// 按键扫描消抖

unsigned char key_statue_buffer = 0;
unsigned char key_statue = 0;
unsigned char key_count = 0;
unsigned char PWM_count = 0;

// 编码器相关变量
unsigned char EC11_State_Old = 0;	 // 编码器上一次状态（A/B相组合）
unsigned char EC11_Debounce_Cnt = 0; // 编码器防抖计数

// Timer1
unsigned int Debounce_count = 0;
unsigned int Timer1_count1 = 0;
unsigned int Timer1_count2 = 0;
unsigned int Timer1_count3 = 0;

// 三个不同时间的旗帜
unsigned char flag_1ms = 0;
unsigned char Seg2_flag = 0;
unsigned char Seg1_flag = 0;

// 温湿度
//  1. 定义全局/局部缓冲区（6个元素，存传感器返回的6字节原始数据）
unsigned int sht_data_buf[6] = {0}; // 缓冲区初始化清零
// 2. 定义变量存最终实际温湿度（浮点型，保留2位小数足够）
float sht_temperature = 0.0f; // 温度(℃)
float sht_humidity = 0.0f;	  // 湿度(%RH)
//===========================================================
//Variable definition
volatile char W_TMP  @ 0x70 ;//系统占用不可以删除和修改
volatile char BSR_TMP  @ 0x71 ;//系统占用不可以删除和修改
void user_isr();//用户中断程序，不可删除
void Delay10Us(void);
void DelayMs(unsigned char Time);
//===========================================================

//===========================================================
//Function name：interrupt ISR
//parameters：无
//returned value：无
//===========================================================
void interrupt ISR(void)
{
#asm;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
	NOP;//系统设置不可以删除和修改
#endasm;//系统设置不可以删除和修改
	user_isr(); //用户中断函数
}
void user_isr() // ????ж????
{
	if (T1UIF)
	{
		T1UIF = 1;
		Timer1_count1++;
		Timer1_count2++;
		Timer1_count3++;
		if (Timer1_count1 >= 20000) // 1s
		{
			flag_1ms = 1;
			Timer1_count1 = 0;
		}
		if (Timer1_count2 >= 3000) // 没有用上
		{
			Seg2_flag = 1;
			Timer1_count2 = 0;
		}
		if (Timer1_count3 >= 3000) // 150ms
		{
			Seg1_flag = 1;
			Timer1_count3 = 0;
		}
	}
}

/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能： 	 上电系统初始化
 * 输入：	 无
 * 输出： 	 无
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // 系统时钟选择为内部振荡器8MHz,分频比为1:1

	PCKEN |= 0B00000010; // 使能定时器1

	INTCON = 0; // 禁止所有中断

	PORTA = 0B00000000;
	PORTB = 0B00000000;
	PORTC = 0B00000000;

	WPUA |= 0B00000000; // EC11_A/EC11_B（编码器）使能弱上拉 // 弱上拉的开关，0-关，1-开
	WPUB = 0B00000000;
	WPUC = 0B00000000;

	WPDA = 0B00000000; // 弱下拉的开关，0-关，1-开
	WPDB = 0B00000000;
	WPDC = 0B00000000;

	TRISA = 0B00000000; // 输入输出设置，0-输出，1-输入 修改为EC11_A、EC11_B为输入
	TRISB = 0B00000010; // PB1输入
	TRISC = 0B00000000;

	PSRC0 = 0B11111111; // 源电流设置最大
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;

	PSINK0 = 0B11111111; // 灌电流设置最大
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;

	ANSELA = 0B00000000; // 设置对应的IO为数字IO
}
void TIM1_INITIAL(void)
{
	PCKEN |= 0B00000010;  // ???TIMER1??????
	CKOCON = 0B00100000;  // Timer1?????????????λ4ns???
	TCKSRC |= 0B00000011; // Timer1?????HIRC??2???

	TIM1CR1 = 0B10000101; // ???????????????????

	TIM1IER = 0B00000001; // ?????????ж?

	TIM1ARRH = 0x06; // ???????8
	TIM1ARRL = 0x40; // ???????8λ  50us ??????ж?

	INTCON = 0B11000000; // ??????ж???????ж?
}

//===========================================================
//Function name：main
//parameters：无
//returned value：无
//===========================================================
void Delay10Us(void)
{
	for(unsigned char i=0;i<2;i++)
	{
		NOP(); 
		NOP();
 		NOP(); 
		NOP(); 
		NOP(); 
		NOP();   
 		NOP(); 
		NOP();  
		NOP(); 
		NOP();                                                                                        
	}
}
 void DelayMs(unsigned char Time)
 {
	for(unsigned int a=0;a<Time;a++)
	{
		for(unsigned b=0;b<96;b++)
		{
		 	Delay10Us(); 	
		}
	}
 }
 void TDelay_us(unsigned int Rt_TM1650) // 这个差不多是输入1，输出3.622的样子
{
	while (Rt_TM1650--)
		;
}
// ms 比较精确1：1
void TDelay_ms(unsigned char Time)
{
	unsigned char a, b;
	for (a = 0; a < Time; a++)
	{
		for (b = 0; b < 5; b++)
		{
			TDelay_us(545);
		}
	}
}
 
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
	unsigned char i;
	txd = (addr_7bit << 1) | 0;
	TRISA &= ~(1 << 4);
	ODCON0 |= ~(1 << 4); // SDA_seg1线输出模式
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
	WPUA = WPUA & ~(1 << 4);   // 关闭若上拉
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
	ODCON0 |= 0B00100000; // SDA_seg1线输出模式
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
		ODCON0 &= ~(1 << 5);
	}

	return 0; // 读取完成
}

void SHT_process(void)
{
	IIC_SHT_Start();
	IIC_SHT_Write_Byte(0x44);
	IIC_SHT_Ack();
	IIC_SHT_Write_Command(0xFD);
	IIC_SHT_Ack();
	IIC_SHT_Stop();
	TDelay_ms(20); // 等待传感器完成传递数据
	IIC_SHT_Start();
	IIC_SHT_Read_Byte(0x44); // 包含了ack
	IIC_SHT_Read_6Bytes(sht_data_buf);
}


main()
{
	POWER_INITIAL();
    TIM1_INITIAL();
    SHT_process();
	while(1)
    {
		if (IIC_SHT_Read_6Bytes(sht_data_buf) == 0)
		{
			// 4. 拼接16位原始数据（高字节+低字节，SHT系列温湿度均为16位数据）
			unsigned int temp_raw = (sht_data_buf[0] << 8) | sht_data_buf[1]; // 温度原始值（第0、1字节）
			unsigned int humi_raw = (sht_data_buf[3] << 8) | sht_data_buf[4]; // 湿度原始值（第3、4字节）
			// （可选）CRC校验：对比sht_data_buf[2]（温度CRC）、sht_data_buf[5]（湿度CRC），提升数据可靠性

			// 5. 转换为实际温湿度（SHT3x/SHT2x通用公式，手册标准转换方法）
			sht_temperature = (float)(temp_raw * 175.0f / 65535.0f) - 45.0f; // 温度公式：-45~125℃
			sht_humidity = (float)(humi_raw * 100.0f / 65535.0f);			 // 湿度公式：0~100%RH
			// 温度
			Number_Sum_1 = sht_temperature;
			// 湿度
			Number_Sum_2 = sht_humidity;
		}

//		if (flag_1ms == 1) // 1s进来执行一次
//		{
//			// 旋钮控制数据
//			// EC11_Process();
//			SHT_process();
//			flag_1ms = 0;
//		}
        
    }
}
//===========================================================
