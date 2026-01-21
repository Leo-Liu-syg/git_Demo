// Project: 1.上电从0开始，一秒向上增加1
// 2.串口接收到0xAA的时候，保存当前值到eeprom
// 3.串口接收到0xBB的时候，设置当前值为eeprom值
// 4.串口接收到0xCC的时候，亮度向上调高一档；接收到0xDD的时候亮度调低一档；
// 5.亮度值要保存到eeprom，上电读取
// Device:  FT64F0AX
// Memory:  PROM=10Kx14, SRAM=1KB, EEPROM=128
// Description: ??????????64F0Ax_IIC?????????.
//
//
// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.6        25-6-4        ?????????8MHz?????LVR
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
// VDD---------|9(VDD)		(PB3)12|------SCL
// NC----------|10(PB5)		(PB4)11|------SDA
//				-------------------

#include "SYSCFG.h"
#include "FT64F0AX.h"
#include "EEPROM.h"
#include "TDelay.h"


// **************************宏定义**************************************
#define TM1650_WRITE_ADDR 0x48
#define TM1650_CMD_DISP_ON 0x01 // 8级亮度，7段显示，开启显示使能（手册好像反了）
#define TM1650_CMD_ADDR_BASE 0x68
#define SCL PB3
#define SDA PB4

#define TM1650_SCL_HOLD_TIME 5 // 单位μs（≥4μs）
#define TM1650_ACK_DELAY 2	   // SCL上升沿后延时
#define ACK_SAMPLE_RETRY 3	   // 采样次数    可以更改

// -----------------------------数码管处理相关---------------------------
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // 0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};

unsigned char Brightness[] = {0x00, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, 0x01};
unsigned char B_Seg_Level = 0;
unsigned char B_Seg_flag = 0;

unsigned int Seg_Count = 0;
unsigned char T_1sFlag = 0;
unsigned int Number_Sum = 0;
unsigned int Number_Ge = 0;
unsigned int Number_Shi = 0;
unsigned int Number_Bai = 0;
unsigned int Number_Qian = 0;

volatile char W_TMP @0x70;	 // ????��?????????????
volatile char BSR_TMP @0x71; // ????��?????????????

// ------------------------------EEPROM相关-----------------------------
volatile unsigned char receivedata = 0;
volatile unsigned char sendaddress = 0;
volatile unsigned char senddata = 0;
volatile unsigned char EEReadData = 0xAA;
unsigned char send_flag = 0;
unsigned char init_send_done = 0;

volatile unsigned char IICReadData;

// -------------------函数声明---------------------
void user_isr(); // ????��???????????
unsigned char EEPROMread(unsigned char EEAddr);
void EEPROMwrite(unsigned char EEAddr, unsigned char Data);
void UART_INITIAL(void);

void interrupt ISR(void)
{
#asm;			// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
	NOP;		// ?????��?????????????
#endasm;		// ?????��?????????????
	user_isr(); // ????????��????
}
void user_isr() // ????????��????
{
	if (TIM1SR1 & 0x01) // 检查更新中断标志位
	{
		TIM1SR1 |= 0x01; // 写1清除T1UIF
		Seg_Count++;
		if (Seg_Count >= 999)
		{
			T_1sFlag = 1;
			Seg_Count = 0;
		}

		// Speed_Flag_1ms = 1; // 每进一次中断置1一次1ms
		// Key_press related
	}

	if (UR1RXNE && UR1RXNEF) // 串口接收from电脑
	{
		UR1RXNEF = 0;
		EEReadData = UR1DATAL;
		// 数字存入+读取
		if (EEReadData == 0xAA)
		{
			EEPROMwrite(0x14, Number_Sum);
		}
		else if (EEReadData == 0xBB)
		{
			Number_Sum = EEPROMread(0x14);
		}

		// 亮度调节
		else if (EEReadData == 0xCC)
		{
			if (B_Seg_Level < 8 && B_Seg_Level >= 0)
			{
				B_Seg_Level++;
			}
			else
			{
				B_Seg_Level = 7;
			}
			EEPROMwrite(0x15, B_Seg_Level);
			B_Seg_flag = 1;
		}
		else if (EEReadData == 0xDD)
		{
			if (B_Seg_Level <= 8 && B_Seg_Level > 0)
			{
				B_Seg_Level--;
			}
			else
			{
				B_Seg_Level = 0;
			}

			EEPROMwrite(0x15, B_Seg_Level);
			B_Seg_flag = 1;
		}

		// 垃圾文件舍不得删
		else if (EEReadData >= 0xff)
		{
			EEPROMwrite(0x13, 0x03);
		}
		else
		{
			EEPROMwrite(0x13, EEReadData);
		}

		send_flag = 1;
	}

	if (UR1TCEN && UR1TCF) // 串口发送回电脑
	{
		UR1TCF = 0;
		if (send_flag == 1)
		{
			senddata = EEPROMread(0x14);
			UR1DATAL = senddata;
			send_flag = 0;
		}
	}
}

void POWER_INITIAL(void)
{
	OSCCON = 0B01100001; // ????????????????8MHz,??????1:1

	INTCON = 0; // ????????��?

	PORTA = 0B00000000;
	PORTB = 0B00000000;
	PORTC = 0B00000000;

	WPUA = 0B00000000; // ????????????0-???1-??
	WPUB = 0B00000000;
	WPUC = 0B00000000;

	WPDA = 0B00000000; // ????????????0-???1-??
	WPDB = 0B00000000;
	WPDC = 0B00000000;

	TRISA = 0B10000000; // PA7写入
	TRISB = 0B00000000; // PB4:IIC-SDA PB3:IIC-SCL
	TRISC = 0B00000000;

	PSRC0 = 0B11111111; // ????????????
	PSRC1 = 0B11111111;
	PSRC2 = 0B00001111;

	PSINK0 = 0B11111111; // ????????????
	PSINK1 = 0B11111111;
	PSINK2 = 0B00000011;

	ANSELA = 0B00000000; // ????????IO?????IO
}
// ********************定时器1**************************
void TIM1_INIT(void) // 1ms进一次中断
{
	PCKEN |= 0B00000010;  // 使能Timer1模块时钟
	CKOCON = 0B00100000;  // 时钟输出配置（不影响中断，保持原配置）
	TCKSRC |= 0B00000011; // Timer1时钟源=2x HIRC（32MHz）

	// 配置预分频器（显式设置1分频，确保计数时钟=32MHz）
	TIM1PSCRH = 0x00;
	TIM1PSCRL = 0x00; // 预分频比=0x0000+1=1

	TIM1CR1 = 0B10000101; // 使能自动重载、边沿对齐、向上计数、使能计数器
	TIM1IER = 0B00000001; // 使能更新中断
	TIM1ARRH = 0x7C;	  // 自动重载值高8位（0x7CFF=31999）
	TIM1ARRL = 0xFF;	  // 自动重载值低8位
	INTCON = 0B11000000;  // 使能全局中断和外设中断
}

// ====================URAT init=======================
void UART_INITIAL(void)
{
	PCKEN |= 0B00100000; // ʹ��UART1ģ��ʱ��
	UR1IER = 0B00100001; // ʹ�ܷ�������ж�+���������ж�
	UR1LCR = 0B00000001; // 8λ���ݣ�1λֹͣλ������żУ��
	UR1MCR = 0B00011000; // ʹ�ܷ��ͺͽ��սӿ�

	UR1DLL = 52; // 16MHz��9600������
	UR1DLH = 0;
	UR1TCF = 1;			 // ɾ�����У���ʼ��ʱ������1��Ӳ�����Զ�����
	INTCON = 0B11000000; // ʹ��ȫ��+�����ж�
}
//====================== IIC ========================
unsigned char IIC_Read(unsigned char address)
{
	unsigned char iicdata = 0;
	while (!IICTXE)
		;
	I2CDR = address;
	I2CCMD = 0B00000110;
	while (!IICTXE)
		;
	I2CCMD = 0B00000011;
	while (!IICRXNE)
		;
	iicdata = I2CDR;
	return iicdata;
}
/***************************************************
**函数名称：IIC_WrByte_TM1650(void)
**函数描述：发送一个字节
**输入    ：None
**输出    ：None
	//SCL高电平期间，SDA产生一个上升沿 表示停止
****************************************************/
void IIC_WrByte_TM1650(unsigned char txd)
{
	unsigned char i;
	ODCON0 |= 0B00000010; // scl SDA线输出模式
	// 定义一个计数变量

	SCL = 0;
	for (i = 0; i < 8; i++)
	{
		if (txd & 0x80)
			SDA = 1;
		else
			SDA = 0;
		txd = txd << 1;
		TDelay_us(2);
		SCL = 1;
		TDelay_us(2);
		SCL = 0;
		TDelay_us(2);
	}
}
void I2C_Start_TM1650(void)
{
	TRISB &= 0B00000000; // 将SDA SCL设置为输出模式
	SDA = 1;
	SCL = 1;
	TDelay_us(5); // 建立时间是 SDA 保持时间>4.7us
	SDA = 0;	  // SDA 从高变低
	TDelay_us(5); // 保持时间是>4us
	SCL = 0;
}
/***************************************************
**函数名称：I2C_Stop_TM1650(void)
**函数描述：结束信号
**输入    ：None
**输出    ：None
	//SCL高电平期间，SDA产生一个上升沿 表示停止
****************************************************/
void I2C_Stop_TM1650(void)
{
	TRISB &= 0B00000000; // SDA SCL设置为输出模式

	SCL = 0;
	SDA = 0; // 保证数据线为低电平
	TDelay_us(2);
	SCL = 1;	  // 先保证时钟线为高电平
	TDelay_us(2); // 延时 以得到一个可靠的电平信号
	SDA = 1;	  // 数据线出现上升沿
	TDelay_us(2); // 延时 保证一个可靠的高电平
}

// ====================== TM1650======================
unsigned char TM1650_IIC_wait_ack(void)
{
	unsigned char ack_signal = 1; // 默认NACK
	SDA = 1;					  // 配置SDA为输入模式 或者是 高阻抗，释放总线

	SCL = 1;						 // 生成SCL上升沿
	TDelay_us(TM1650_SCL_HOLD_TIME); // 保持SCL高电平

	// 在稳定窗口期内多次采样
	TDelay_us(TM1650_ACK_DELAY);
	for (int i = 0; i < ACK_SAMPLE_RETRY; i++)
	{
		if (SDA == 0) // 获得SDA的高低电平 按目前写的程序是直接写成：if（SDA ==0）
		{
			ack_signal = 0; // 检测到ACK
			break;
		}
		TDelay_us(1); // 采样间隔
	}

	SCL = 0; // 结束时钟脉冲
	TDelay_us(TM1650_SCL_HOLD_TIME);
	return ack_signal;
}
/***************************************************
**函数名称：TM1650_cfg_display()
**函数描述：设置显示参数
**输入    ：None
**输出    ：None
设置亮度并打开显示: TM1650_BRIGHTx  0x79
关闭显示 TM1650_DSP_OFF 0x00
****************************************************/
void TM1650_cfg_display(unsigned char param)
{
	I2C_Start_TM1650();
	IIC_WrByte_TM1650(0x48); // 设置系统参数命令
	TM1650_IIC_wait_ack();
	IIC_WrByte_TM1650(param); // 系统参数设置
	TM1650_IIC_wait_ack();
	I2C_Stop_TM1650();
}
/***************************************************
**函数名称：TM1650_Set(unsigned char  add,unsigned char dat)
**函数描述：数码管显示
**输入    ：None
**输出    ：None
add ：位地址
dat ：显示的数值
****************************************************/
void TM1650_Set(unsigned char add, unsigned char dat) // 数码管显示
{
	I2C_Start_TM1650();
	IIC_WrByte_TM1650(add);
	TM1650_IIC_wait_ack(); // 显存起始地址为0x68
	IIC_WrByte_TM1650(dat);
	TM1650_IIC_wait_ack(); // 发送段码

	I2C_Stop_TM1650();
}
/***************************************************
**函数名称：TM1650_DisplayOpen(void)
**函数描述：数码管显示
**输入    ：None
**输出    ：None
设置亮度并打开显示: TM1650_BRIGHTx  0x79
****************************************************/
void TM1650_DisplayOpen(void)
{
	TM1650_cfg_display(0x87);
}
/***************************************************
**函数名称：TM1650_displayClose(void)
**函数描述：数码管关闭显示
**输入    ：None
**输出    ：None
关闭显示 TM1650_DSP_OFF 0x00
****************************************************/
void TM1650_DisplayClose(void)
{
	TM1650_cfg_display(0x81);
	TRISB &= 0B00011000; // SCL SDA输入模式
}

void TM1650_Init(void)
{
	// ??????????????0x8F???????+7???????
	TM1650_Set(TM1650_WRITE_ADDR, TM1650_CMD_DISP_ON);
	TDelay_ms(5); // ???????????��
}
/*======================== main ==========================*/
void main(void)
{
	POWER_INITIAL();

	//-----数码管+定时器初始化------
	TM1650_Init();
	TIM1_INIT();

	//-----EEPROM初始化------------
	EEPROMwrite(0x13, 0x55);	   // 0x55写入地址0x13
	EEReadData = EEPROMread(0x13); // 芯片上电的时候把EEPROM 0x13房间里的内容通过串口显示出来
	Number_Sum = EEPROMread(0x14); //读取数字
	B_Seg_Level =EEPROMread(0x15); //读取亮度
	TM1650_cfg_display(Brightness[B_Seg_Level]);

	UART_INITIAL(); // 使能串口，目前来看必须在write弄完了之后再开启中断，否则会无法解锁
	TDelay_ms(100); // 延迟一会，等待串口ok
	UR1TCF = 0;
	UR1DATAL = EEReadData;

	while (1)
	{
		// R_condition = 1;

		if (T_1sFlag == 1 && Number_Sum < 10)
		{
			TM1650_Set(led_place[0], seg_code[Number_Sum]);
			TM1650_Set(led_place[1], 0);
			TM1650_Set(led_place[2], 0);
			TM1650_Set(led_place[3], 0);
			Number_Sum++;
			T_1sFlag = 0;
		}
		else if (T_1sFlag == 1 && (Number_Sum >= 10 && Number_Sum < 100))
		{
			Number_Ge = Number_Sum % 10;
			Number_Shi = Number_Sum / 10;
			TM1650_Set(led_place[0], seg_code[Number_Shi]);
			TM1650_Set(led_place[1], seg_code[Number_Ge]);
			TM1650_Set(led_place[2], 0);
			TM1650_Set(led_place[3], 0);

			Number_Sum++;
			T_1sFlag = 0;
		}
		else
		{
		}
		// 调节亮度
		if (B_Seg_flag == 1)
		{
			TM1650_cfg_display(Brightness[B_Seg_Level]);
			B_Seg_flag = 0;
		}

		if (Number_Sum >= 100)
		{
			Number_Sum = 0;
		}
	}
}
