// Project: TEST_64F0Ax_IIC.prj
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
//
// *********************************************************

#include "SYSCFG.h"
#include "FT64F0AX.h"

#define TM1650_WRITE_ADDR 0x48
#define TM1650_CMD_DISP_ON 0x01 // 8级亮度，7段显示，开启显示使能（手册好像反了）
#define TM1650_CMD_ADDR_BASE 0x68
#define SCL PB3
#define SDA PB4

#define TM1650_SCL_HOLD_TIME 5 // 单位μs（≥4μs）
#define TM1650_ACK_DELAY 2	   // SCL上升沿后延时
#define ACK_SAMPLE_RETRY 3	   // 采样次数    可以更改

// LED处理相关
unsigned char seg_code[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};//0~9
unsigned char led_place[] = {0x68, 0x6A, 0x6C, 0x6E};

volatile char W_TMP @0x70;	 // ????��?????????????
volatile char BSR_TMP @0x71; // ????��?????????????

volatile unsigned char R_condition = 0;
volatile unsigned char C_condition_OFF = 0;
volatile unsigned char F_flash = 0;
volatile unsigned char R8_address = 0;
volatile unsigned char R8_led;
volatile unsigned char R8_led_place = 0;
volatile unsigned char R8_led_seg = 0;

volatile unsigned char IICReadData;
void user_isr(); // ????��???????????
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

	TRISA = 0B00000000; // ????????????0-?????1-????
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

// void IIC_INITIAL(void)
// {

// 	PCKEN |= 0B01000000;  // ???I2C??????
// 	ODCON0 |= 0B00000010; // I2C_SCL,I2C_SDA????????????????��

// 	I2CFREQ = 0B00010000; // ???????????16MHz

// 	I2CCR1 = 0B00000001; // ???????????????100KHz????7��??????
// 	I2CCR2 = 0B00000000;
// 	I2CCR3 = 0B00000000;  // ????I2C??�????��???????????????????????????SCL???????
// 	I2COARL = 0B01010000; // ??????
// 	I2COARH = 0B00000000;

// 	I2CCCRL = 0B01010000; // ??????��?SCL???????? ??????????/(2*CCR)
// 	I2CCCRH = 0B00000000;
// 	I2CITR = 0B00000000; // ?????????��?
// 	ENABLE = 1;			 // ????I2C
// }

// us_Dealy
void TDelay_us(unsigned char Rt_TM1650)

{
	while (Rt_TM1650--)
	{
		NOP();
		NOP();
		NOP();
	}
}

void TDelay_ms(unsigned char Time)
{
	unsigned char a, b;
	for (a = 0; a < Time; a++)
	{
		for (b = 0; b < 5; b++)
		{
			TDelay_us(197);
		}
	}
}

void TDelay_s(unsigned char Time)
{
	unsigned char a, b;
	for (a = 0; a < Time; a++)
	{
		for (b = 0; b < 10; b++)
		{
			TDelay_ms(100);
		}
	}
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

// void TM1650_DisplayNum(unsigned char pos, unsigned char num)
// {
// 	// 4��??????????????TM1650???�Z??
// 	unsigned char disp_addr[] = {0x68, 0x6A, 0x6C, 0x6E};
// 	if (pos < 1 || pos > 4 || num > 9)
// 		return; // ?????

// 	// ????????????????????????��???
// 	IIC_Write(TM1650_WRITE_ADDR, disp_addr[pos - 1]);
// 	TDelay_us(50);
// 	// ?????????????????????
// 	IIC_Write(TM1650_WRITE_ADDR, seg_code[num]);
// }

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
	IIC_WrByte_TM1650(0x48);  // 设置系统参数命令
	IIC_WrByte_TM1650(param); // 系统参数设置
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
/*-----------------------------------------------------------
* 函数： LED_DEAL( )
* 功能： 显示的处理函数，10ms执行一次
* 输入： 无
* 输出： 无
------------------------------------------------------------*/
void LED_DEAL(void) // 先初始化了亮度和显示才可以用
{
	if (R_condition != C_condition_OFF) // 开机,上电时软件置1
	{
		//==========================显示数据=====================================
		I2C_Start_TM1650();
		if (F_flash == 0)
		{
			if (R8_address == 0) // 个位
			{
				R8_led_place = led_place[0];		  // 这个要处理一下逻辑。LED——place代指什么--done
				R8_led_seg = seg_code[1];			  // 1
				TM1650_Set(R8_led_place, R8_led_seg); // 显示dig1
				R8_address++;
			}
			else if (R8_address == 1) // 十位
			{
				R8_led_place = led_place[1];
				if (seg_code[1] < 10)
				{
					R8_led_seg = 0x00;
				}
				else
				{
					R8_led_seg = seg_code[1]; // 2
				}

				TM1650_Set(R8_led_place, R8_led_seg); // 显示dig2
				R8_address++;
			}
			else if (R8_address == 2) // 百位
			{
				R8_led_place = led_place[2];

				if (seg_code[2] < 100)
				{
					R8_led_seg = 0x00;
				}
				else
				{
					R8_led_seg = seg_code[2]; // 3
				}
				TM1650_Set(R8_led_place, R8_led_seg); // 显示dig3
				R8_address = 0;
			}
		}
		else
		{ // 关机
			TM1650_DisplayClose();
		}
	}
	else
	{
		//==========================数据=====================================
		TM1650_DisplayClose(); // 关机

		R8_address = 0;
		F_flash = 0;
	}
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
	POWER_INITIAL(); // ???????
	// IIC_INITIAL();
	TM1650_Init();
	// TM1650_Init();	 // ??????TM1650????????????????

	// ??????????1��???????1???????pos=2/3/4??num=0-9??
	// TM1650_DisplayNum(1, 1);

	while (1)
	{
		R_condition = 1;
		LED_DEAL();
	}
}

