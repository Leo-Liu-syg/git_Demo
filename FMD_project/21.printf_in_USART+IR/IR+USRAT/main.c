#include "SYSCFG.h"
#include "FT64F0AX.h"

#define LED PB3
#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long

unsigned char flag_1 = 0;

volatile u8 ir_parse_ok = 0;
volatile u8 ir_key_code = 0;

// 红外全局变量：替换32位ir_data为4个8位字节数组（核心修改）
volatile u8 ir_state = 0;
volatile u16 ir_t100us_count = 0;
volatile u8 ir_bit_cnt = 0;
// 拆分为4个8位变量，对应32位数据的4个字节：[0]地址码 [1]地址反码 [2]命令码 [3]命令反码
volatile u8 ir_data_byte[4] = {0, 0, 0, 0};
volatile u8 ir_key_ready = 0;
volatile u8 ir_key_value = 0;
volatile u8 ir_addr = 0;

volatile u8 addr = 0;     // 地址码（第0个字节）
volatile u8 addr_inv = 0;// 地址反码（第1个字节）
volatile u8 cmd = 0;     // 命令码（第2个字节）
volatile u8 cmd_inv = 0;

void user_isr();
void interrupt ISR(void)
{
#asm;           // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
    NOP;        // ϵͳ���ò�����ɾ�����޸�
#endasm;        // ϵͳ���ò�����ɾ�����޸�
    user_isr(); // �����û��жϺ���
}
void user_isr() // �����û��жϺ���
{
    // Timer1
    if (T1UIE && T1UIF) // 如果发生更新事件T1UIF
    {
        T1UIF = 1;
        flag_1 = 1;
        ir_t100us_count++;
    }
}

void IR_Edge_Process(void)
{
    switch (ir_state)
    {
    case 0: // 空闲等待 - 检测引导码起始低电平
        if (PA0 == 0)
        {
            ir_t100us_count = 0;
            ir_state = 1;
        }
        break;

    case 1: // 检测引导码9ms低电平
        if (ir_t100us_count > 80 && ir_t100us_count < 95)
        {
            if (PA0 == 1)
            {
                ir_t100us_count = 0;
                ir_state = 2;
            }
        }
        else if (ir_t100us_count > 100) // 超时复位
        {
            ir_state = 0;
            ir_t100us_count = 0;
        }
        break;

    case 2: // 检测引导码高电平 - 区分正常码/重复码（放宽范围）
        // 正常码4.5ms高电平
        if (ir_t100us_count > 40 && ir_t100us_count < 50)
        {
            if (PA0 == 0)
            {
                ir_t100us_count = 0;
                ir_bit_cnt = 0;
                // 修正：初始化4个8位字节（替代原ir_data=0）
                ir_data_byte[0] = 0;
                ir_data_byte[1] = 0;
                ir_data_byte[2] = 0;
                ir_data_byte[3] = 0;
                ir_state = 3;
            }
        }
        // 重复码2.25ms高电平（18~26）
        else if (ir_t100us_count > 18 && ir_t100us_count < 26)
        {
            if (PA0 == 0)
            {
                ir_key_ready = 1;
                ir_state = 0;
                ir_t100us_count = 0;
            }
        }
        else if (ir_t100us_count > 60) // 超时复位
        {
            ir_state = 0;
            ir_t100us_count = 0;
        }
        break;

    case 3: // 解析数据位低电平
        if (ir_t100us_count > 3 && ir_t100us_count < 7)
        {
            if (PA0 == 1)
            {
                ir_t100us_count = 0;
                ir_state = 4;
            }
        }
        else if (ir_t100us_count > 10) // 超时复位
        {
            ir_state = 0;
            ir_t100us_count = 0;
        }
        break;

    case 4: // 解析数据位高电平，区分0/1（核心修改：8位字节的位赋值）
        // 逻辑0：562.5μs高电平
        if (ir_t100us_count > 3 && ir_t100us_count < 8)
        {
            if (PA0 == 0)
            {
                // 逻辑0：无需赋值（字节默认0），仅计数+1
                ir_bit_cnt++;
                ir_t100us_count = 0;
                ir_state = 3;
            }
        }
        // 逻辑1：1687.5μs高电平（12~22）
        else if (ir_t100us_count > 14 && ir_t100us_count < 20)
        {
            if (PA0 == 0)
            {
                // 核心修改：给对应8位字节的对应位赋值1（避开32位移位）
                u8 byte_idx = ir_bit_cnt / 8;             // 计算当前位属于第几个8位字节（0~3）
                u8 bit_pos = ir_bit_cnt % 8;              // 计算当前位在字节内的位置（0~7）
                ir_data_byte[byte_idx] |= (1 << bit_pos); // 8位移位，编译器完全兼容

                ir_bit_cnt++;
                ir_t100us_count = 0;
                ir_state = 3;
            }
        }
        else if (ir_t100us_count > 25) // 超时复位
        {
            ir_state = 0;
            ir_t100us_count = 0;
        }

        // 检测是否解析完32位数据
        if (ir_bit_cnt >= 32)
        {
            // 修正：直接从8位字节提取地址、命令（替代原32位移位）
            addr = ir_data_byte[0];     // 地址码（第0个字节）
            addr_inv = ir_data_byte[1]; // 地址反码（第1个字节）
            cmd = ir_data_byte[2];      // 命令码（第2个字节）
            cmd_inv = ir_data_byte[3];  // 命令反码（第3个字节）

            // 反码验证
            if (cmd == (u8)(~cmd_inv))
            {
                ir_addr = addr;
                ir_key_value = cmd;
                ir_key_ready = 1;
            }

            // 复位状态机
            ir_state = 0;
            ir_t100us_count = 0;
            ir_bit_cnt = 0;
            // 复位4个8位字节（替代原ir_data=0）
            ir_data_byte[0] = 0;
            ir_data_byte[1] = 0;
            ir_data_byte[2] = 0;
            ir_data_byte[3] = 0;
        }
        break;

    default: // 异常复位
        ir_state = 0;
        ir_t100us_count = 0;
        ir_bit_cnt = 0;
        // 复位4个8位字节
        ir_data_byte[0] = 0;
        ir_data_byte[1] = 0;
        ir_data_byte[2] = 0;
        ir_data_byte[3] = 0;
        break;
    }
}

void TIM1_Init(void) // 100us
{
    PCKEN |= 0B00000010;
    CKOCON = 0B00100000;
    TCKSRC = 0B00000011;

    TIM1CR1 = 0B10000101;

    TIM1IER = 0B00000001;

    TIM1ARRH = 0x0C; // �Զ�װ�ظ�8λH
    TIM1ARRL = 0x7E; // �Զ�װ�ص�8λH

    INTCON = 0B11000000;
}

void EXTI_Init(void)
{
    TRISA |= 0x01;   // PA0设为输入
    ANSELA &= ~0x01; // 禁用模拟功能，启用数字输入
    WPUA |= 0x01;    // 开启弱上拉，稳定空闲高电平
}
void main(void)
{

    TRISB = 0x00; // LED输出
    TIM1_Init();
    EXTI_Init();

    INTCON |= 0xC0; // 开启全局中断（GIE=1）+ 外设中断（PEIE=1）
    ir_state = 0;
    ir_t100us_count = 0;
    ir_bit_cnt = 0;
    ir_key_ready = 0;
    ir_key_value = 0;

    while (1)
    {

        if (flag_1 == 1)
        {
            flag_1 = 0;
            PB0 = ~PB0;
        }
        IR_Edge_Process();
    }
}