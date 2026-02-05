
#include	"SYSCFG.h"
#include 	"FT64F0AX.h"
#include    "TDelay.h"

#define TM1650_WRITE_ADDR 0x48
#define TM1650_CMD_DISP_ON 0x01 // 8级亮度，7段显示，开启显示使能（手册好像反了）
#define TM1650_CMD_ADDR_BASE 0x68
#define SCL_seg1 PB3
#define SDA_seg1 PB4

#define TM1650_SCL_seg1_HOLD_TIME 5 // 单位μs（≥4μs）
#define TM1650_ACK_DELAY 2	   // SCL_seg1上升沿后延时
#define ACK_SAMPLE_RETRY 3	   // 采样次数    可以更改
/* ************************I2C_stimulated*************************** */
void IIC_WrByte_TM1650_1(unsigned char txd)//有用到
{
	unsigned char i;
	ODCON0 |= 0B00000010; // scl_seg1 SDA_seg1线输出模式
	// 定义一个计数变量

	SCL_seg1 = 0;
	for (i = 0; i < 8; i++)
	{
		if (txd & 0x80)
			SDA_seg1 = 1;
		else
			SDA_seg1 = 0;
		txd = txd << 1;
		TDelay_us(2);
		SCL_seg1 = 1;
		TDelay_us(2);
		SCL_seg1 = 0;
		TDelay_us(2);
	}
}
void I2C_Start_TM1650_1(void)//有用到
{
	// TRISB &= 0B00000000; // 将SDA_seg1 SCL_seg1设置为输出模式
	SDA_seg1 = 1;
	SCL_seg1 = 1;
	TDelay_us(5); // 建立时间是 SDA_seg1 保持时间>4.7us
	SDA_seg1 = 0;	  // SDA_seg1 从高变低
	TDelay_us(5); // 保持时间是>4us
	SCL_seg1 = 0;
}
/***************************************************
**函数名称：I2C_Stop_TM1650(void)
**函数描述：结束信号
**输入    ：None
**输出    ：None
	//SCL_seg1高电平期间，SDA_seg1产生一个上升沿 表示停止
****************************************************/
void I2C_Stop_TM1650_1(void)//有用到
{
	// TRISB &= 0B00000000; // SDA_seg1 SCL_seg1设置为输出模式
	SCL_seg1 = 0;
	SDA_seg1 = 0; // 保证数据线为低电平
	TDelay_us(2);
	SCL_seg1 = 1;	  // 先保证时钟线为高电平
	TDelay_us(2); // 延时 以得到一个可靠的电平信号
	SDA_seg1 = 1;	  // 数据线出现上升沿
	TDelay_us(2); // 延时 保证一个可靠的高电平
}

// ====================== TM1650======================
unsigned char TM1650_1_IIC_wait_ack(void)//有用到
{
	unsigned char ack_signal = 1; // 默认NACK
	SDA_seg1 = 1;					  // 配置SDA_seg1为输入模式 或者是 高阻抗，释放总线
	SCL_seg1 = 1;						 // 生成SCL_seg1上升沿
	TDelay_us(TM1650_SCL_seg1_HOLD_TIME); // 保持SCL_seg1高电平

	// 在稳定窗口期内多次采样
	TDelay_us(TM1650_ACK_DELAY);
	for (int i = 0; i < ACK_SAMPLE_RETRY; i++)
	{
		if (SDA_seg1 == 0) // 获得SDA_seg1的高低电平 按目前写的程序是直接写成：if（SDA_seg1 ==0）
		{
			ack_signal = 0; // 检测到ACK
			break;
		}
		TDelay_us(1); // 采样间隔
	}

	SCL_seg1 = 0; // 结束时钟脉冲
	TDelay_us(TM1650_SCL_seg1_HOLD_TIME);
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
void TM1650_1_cfg_display(unsigned char param)//设置亮度会用
{
	I2C_Start_TM1650_1();
	IIC_WrByte_TM1650_1(0x48); // 设置系统参数命令
	TM1650_1_IIC_wait_ack();
	IIC_WrByte_TM1650_1(param); // 系统参数设置
	TM1650_1_IIC_wait_ack();
	I2C_Stop_TM1650_1();
}
/***************************************************
**函数名称：TM1650_1_Set(unsigned char  add,unsigned char dat_seg1)
**函数描述：数码管显示
**输入    ：None
**输出    ：None
add ：位地址
dat_seg1 ：显示的数值
****************************************************/
void TM1650_1_Set(unsigned char add, unsigned char dat_seg1) // 有用到数码管显示
{
	I2C_Start_TM1650_1();
	IIC_WrByte_TM1650_1(add);
	TM1650_1_IIC_wait_ack(); // 显存起始地址为0x68
	IIC_WrByte_TM1650_1(dat_seg1);
	TM1650_1_IIC_wait_ack(); // 发送段码

	I2C_Stop_TM1650_1();
}


void TM1650_1_Init(void)//有用到
{
	// ??????????????0x8F???????+7???????
	TM1650_1_Set(TM1650_WRITE_ADDR, TM1650_CMD_DISP_ON);
	TDelay_ms(5); // ???????????��
}

