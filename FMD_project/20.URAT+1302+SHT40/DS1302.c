#include "SYSCFG.h"
#include "FT64F0AX.h"

//***********************宏定义****************************
// #define		uchar	unsigned char
// 1. 引脚宏定义（依据：手册2.3.3 TRISB寄存器、2.3.6 PORTB寄存器）
#define DS1302_SCLK_PIN 0 // PB0
#define DS1302_DATA_PIN 1 // PB1
#define DS1302_RST_PIN 2  // PB2

// 时间变量定义
unsigned char ds1302_year = 26;    // 默认2024年
unsigned char ds1302_month = 12;   // 默认12月
unsigned char ds1302_day = 31;     // 默认31日
unsigned char ds1302_hour = 23;    // 默认23时
unsigned char ds1302_minute = 59;  // 默认59分
unsigned char ds1302_second = 0;   // 默认0秒
unsigned char ds1302_week = 7;     // 默认周六（7）

// 寄存器地址宏定义
#define DS1302_SEC_ADDR   0x80  // 秒寄存器地址
#define DS1302_MIN_ADDR   0x82  // 分寄存器地址
#define DS1302_HOUR_ADDR  0x84  // 时寄存器地址
#define DS1302_WEEK_ADDR  0x86  // 周寄存器地址
#define DS1302_DAY_ADDR   0x88  // 日寄存器地址
#define DS1302_MON_ADDR   0x8A  // 月寄存器地址
#define DS1302_YEAR_ADDR  0x8C  // 年寄存器地址
#define DS1302_WP_ADDR    0x8E  // 写保护寄存器地址

// I/O操作宏定义（依据：手册2.3.3 TRISB(0x8D)、2.3.6 PORTB(0x0D)、2.3.9 LATB(0x10D)）
#define TRISB_REG         (*(volatile unsigned char *)0x8D)  // PORTB方向寄存器
#define PORTB_REG         (*(volatile unsigned char *)0x0D)  // PORTB数据寄存器
#define LATB_REG          (*(volatile unsigned char *)0x10D) // PORTB锁存器

// 方向配置：1=输入，0=输出
#define SET_SCLK_OUT()    (TRISB_REG &= ~(1 << DS1302_SCLK_PIN))
#define SET_DATA_OUT()    (TRISB_REG &= ~(1 << DS1302_DATA_PIN))
#define SET_DATA_IN()     (TRISB_REG |= (1 << DS1302_DATA_PIN))
#define SET_RST_OUT()     (TRISB_REG &= ~(1 << DS1302_RST_PIN))

// 电平操作
#define SCLK_HIGH()       (LATB_REG |= (1 << DS1302_SCLK_PIN))
#define SCLK_LOW()        (LATB_REG &= ~(1 << DS1302_SCLK_PIN))
#define DATA_HIGH()       (LATB_REG |= (1 << DS1302_DATA_PIN))
#define DATA_LOW()        (LATB_REG &= ~(1 << DS1302_DATA_PIN))
#define DATA_READ()       ((PORTB_REG & (1 << DS1302_DATA_PIN)) ? 1 : 0)
#define RST_HIGH()        (LATB_REG |= (1 << DS1302_RST_PIN))
#define RST_LOW()         (LATB_REG &= ~(1 << DS1302_RST_PIN))

// BCD转十进制
static unsigned char BCD2DEC(unsigned char bcd)
{
    unsigned char dec;
    dec = (bcd / 16) * 10;
    dec += (bcd % 16);
    return dec;
}

// 十进制转BCD
static unsigned char DEC2BCD(unsigned char dec)
{
    unsigned char bcd;
    bcd = (dec / 10) * 16;
    bcd += (dec % 10);
    return bcd;
}

// 私有函数：写1字节数据
static void DS1302_WriteByte(unsigned char addr, unsigned char data)
{
    unsigned char i;
    RST_LOW();
    SCLK_LOW();
    RST_HIGH();  // 启动通信（依据：DS1302芯片时序规范）

    // 写地址（低位先行）
    for(i = 0; i < 8; i++)
    {
        if(addr & 0x01)
        {
            DATA_HIGH();
        }
        else
        {
            DATA_LOW();
        }
        SCLK_HIGH();  // 时钟上升沿锁存
        SCLK_LOW();
        addr >>= 1;
    }

    // 写数据（低位先行）
    for(i = 0; i < 8; i++)
    {
        if(data & 0x01)
        {
            DATA_HIGH();
        }
        else
        {
            DATA_LOW();
        }
        SCLK_HIGH();  // 时钟上升沿锁存
        SCLK_LOW();
        data >>= 1;
    }

    RST_LOW();  // 结束通信
}

// 私有函数：读1字节数据
static unsigned char DS1302_ReadByte(unsigned char addr)
{
    unsigned char i, data = 0;
    RST_LOW();
    SCLK_LOW();
    RST_HIGH();  // 启动通信

    // 写地址（读地址=写地址+1）
    for(i = 0; i < 8; i++)
    {
        if(addr & 0x01)
        {
            DATA_HIGH();
        }
        else
        {
            DATA_LOW();
        }
        SCLK_HIGH();
        SCLK_LOW();
        addr >>= 1;
    }

    SET_DATA_IN();  // 切换DATA为输入（依据：手册2.3.3 TRISB寄存器配置）
    data = 0;
    // 读数据（低位先行）
    for(i = 0; i < 8; i++)
    {
        data >>= 1;
        if(DATA_READ() == 1)
        {
            data |= 0x80;
        }
        else
        {
            data &= 0x7F;
        }
        SCLK_HIGH();  // 时钟上升沿读取
        SCLK_LOW();
    }

    SET_DATA_OUT(); // 恢复DATA为输出
    RST_LOW();     // 结束通信
    return data;
}

// 初始化函数
void DS1302_Init(void)
{
    // 1. 配置PB0/PB1/PB2为输出
    SET_SCLK_OUT();
    SET_DATA_OUT();
    SET_RST_OUT();

    // 2. 初始电平配置
    SCLK_LOW();
    RST_LOW();
    DATA_LOW();

    // 3. 关闭写保护（DS1302必须步骤）
    DS1302_WriteByte(DS1302_WP_ADDR, 0x00);
}

// 写入时间
void DS1302_WriteTime(void)
{
    DS1302_WriteByte(DS1302_WP_ADDR, 0x00);  // 关闭写保护

    // 写入秒（屏蔽最高位，确保时钟运行）
    DS1302_WriteByte(DS1302_SEC_ADDR, DEC2BCD(ds1302_second) & 0x7F);
    // 写入分
    DS1302_WriteByte(DS1302_MIN_ADDR, DEC2BCD(ds1302_minute));
    // 写入时（24小时制）
    DS1302_WriteByte(DS1302_HOUR_ADDR, DEC2BCD(ds1302_hour) & 0x3F);
    // 写入周
    DS1302_WriteByte(DS1302_WEEK_ADDR, DEC2BCD(ds1302_week));
    // 写入日
    DS1302_WriteByte(DS1302_DAY_ADDR, DEC2BCD(ds1302_day));
    // 写入月
    DS1302_WriteByte(DS1302_MON_ADDR, DEC2BCD(ds1302_month));
    // 写入年
    DS1302_WriteByte(DS1302_YEAR_ADDR, DEC2BCD(ds1302_year));

    DS1302_WriteByte(DS1302_WP_ADDR, 0x80);  // 开启写保护
}

// 读取时间（无结构体，直接赋值给独立变量）
void DS1302_ReadTime(void)
{
    ds1302_second = BCD2DEC(DS1302_ReadByte(DS1302_SEC_ADDR + 1) & 0x7F);
    ds1302_minute = BCD2DEC(DS1302_ReadByte(DS1302_MIN_ADDR + 1));
    ds1302_hour   = BCD2DEC(DS1302_ReadByte(DS1302_HOUR_ADDR + 1) & 0x3F);
    ds1302_week   = BCD2DEC(DS1302_ReadByte(DS1302_WEEK_ADDR + 1));
    ds1302_day    = BCD2DEC(DS1302_ReadByte(DS1302_DAY_ADDR + 1));
    ds1302_month  = BCD2DEC(DS1302_ReadByte(DS1302_MON_ADDR + 1));
    ds1302_year   = BCD2DEC(DS1302_ReadByte(DS1302_YEAR_ADDR + 1));
}